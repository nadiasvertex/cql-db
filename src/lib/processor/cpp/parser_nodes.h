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
};

typedef std::unique_ptr<node> node_handle_type;

typedef std::stack<node_handle_type> node_list_type;

/**
 * Binary operation node.
 */
class binop: public node
{
	node_handle_type left;
	node_handle_type right;
public:
	binop(node::node_type _type, node* _left, node* _right) :
			node(_type), left(_left), right(_right)
	{
	}

	virtual ~binop()
	{
	}

	/**
	 * Get the left node.
	 */
	node* get_left()
	{
		return left.get();
	}

	/**
	 * Get the right node.
	 */
	node* get_right()
	{
		return right.get();
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
	std::string name;
public:
	column_ref(const std::string& n) :
			node(node::node_type::COLUMN_REF), name(n)
	{
	}

	virtual ~column_ref()
	{
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

	/**
	 * Sets the column reference.
	 */
	void set_column_ref(column_ref *cr)
	{
		col_ref = std::unique_ptr < column_ref > (cr);
	}
};

} // namespace actions
} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_PARSER_NODES_H__
