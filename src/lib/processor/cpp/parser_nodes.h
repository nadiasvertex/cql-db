/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_PROCESSOR_PARSER_NODES_H__
#define __LATTICE_PROCESSOR_PARSER_NODES_H__

#include <memory>
#include <stack>

#include <cell/cpp/data_value.h>

#include <pegtl.hh>

namespace lattice {
namespace processor {
namespace actions {

using namespace pegtl;

/**
 * Base class for nodes.
 */
class node
{
public:
   enum class node_type
   {
      ROOT,
      OP_ADD,
      OP_SUB,
      OP_MUL,
      OP_DIV,
      OP_MOD,
      OP_CAT,
      COLUMN_REF,
      TABLE_REF,
      LITERAL
   };

private:
   node_type type;

public:
   node(node_type _type) :
         type(_type)
   {
   }

   virtual ~node()
   {
   }
   ;

   /**
    * Provides the type of the node.
    */
   node_type get_type()
   {
      return type;
   }

   /**
    * Visit each node in the tree once. Call fn with each node
    * as the parameter.
    *
    * @param fn: The function to execute on each node.
    *
    * Performs a left-first, depth-first traversal.
    */
   void visit(std::function<void(node*)> fn);

   template<typename T>
   T visit_mr(std::function<T(node*)> map,
         std::function<T(const T&, const T&)> reduce);
};

typedef std::shared_ptr<node> node_handle_type;

typedef std::stack<node_handle_type> node_list_type;

/**
 * Binary operation node.
 */
class binop: public node
{
   node_handle_type left;
   node_handle_type right;
public:
   binop(node::node_type _type, node_handle_type _left, node_handle_type _right) :
         node(_type), left(_left), right(_right)
   {
   }

   virtual ~binop()
   {
   }

   /**
    * Get the left node.
    */
   node_handle_type get_left()
   {
      return left;
   }

   /**
    * Get the right node.
    */
   node_handle_type get_right()
   {
      return right;
   }
};

/**
 * Literal node.
 */
class literal: public node
{
   cell::data_value value;
public:
   literal(const cell::data_value& v) :
         node(node::node_type::LITERAL), value(v)
   {
   }

   virtual ~literal()
   {
   }

   const cell::data_value& get_value() const
   {
      return value;
   }
};

/**
 * Column reference.
 */
class column_ref: public node
{
   int column_index;

   std::string name;
public:
   column_ref(const std::string& n, int index) :
         node(node::node_type::COLUMN_REF), name(n), column_index(index)
   {
   }

   virtual ~column_ref()
   {
   }

   std::string& get_name()
   {
      return name;
   }

   int get_index()
   {
      return column_index;
   }
};

/**
 * Table reference.
 */
class table_ref: public node
{
   std::string name;

   std::unique_ptr<column_ref> col_ref;

public:
   table_ref(const std::string& n) :
         node(node::node_type::TABLE_REF), name(n), col_ref(nullptr)
   {
   }

   virtual ~table_ref()
   {
   }

   std::string& get_name()
   {
      return name;
   }

   /**
    * Sets the column reference.
    */
   void set_column_ref(column_ref *cr)
   {
      col_ref = std::unique_ptr<column_ref>(cr);
   }

   column_ref* get_column_ref()
   {
      return col_ref.get();
   }
};

inline void node::visit(std::function<void(node*)> fn)
{
   switch (get_type())
      {
      case node_type::OP_ADD:
      case node_type::OP_SUB:
      case node_type::OP_MUL:
      case node_type::OP_DIV:
      case node_type::OP_MOD:
         {
            auto* bn = dynamic_cast<binop*>(this);
            if (bn != nullptr)
               {
                  bn->get_left()->visit(fn);
                  bn->get_right()->visit(fn);
               }
         }
      break;
      }

   fn(this);
}

template<typename T>
inline T node::visit_mr(std::function<T(node*)> map,
      std::function<T(const T&, const T&)> reduce)
{
   switch (get_type())
      {
      case node_type::OP_ADD:
      case node_type::OP_SUB:
      case node_type::OP_MUL:
      case node_type::OP_DIV:
      case node_type::OP_MOD:
         {
            auto* bn = dynamic_cast<binop*>(this);
            if (bn != nullptr)
               {
                  auto l = bn->get_left()->visit_mr(map, reduce);
                  auto r = bn->get_right()->visit_mr(map, reduce);

                  return reduce(l, r);
               }
         }
      break;

      default:
         return map(this);
      }
}

} // namespace actions
} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_PARSER_NODES_H__
