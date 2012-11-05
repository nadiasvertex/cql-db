/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_CELL_PARSER_ACTIONS_H__
#define __LATTICE_CELL_PARSER_ACTIONS_H__

#include <algorithm>
#include <memory>
#include <stack>
#include <vector>

#include <cell/cpp/data_value.h>
#include <cell/cpp/predicate.h>
#include <cell/cpp/database.h>

#include <pegtl.hh>

namespace lattice
{
namespace cell
{

namespace actions
{

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
	data_value value;
public:
	literal(const data_value& v) :
			node(node::node_type::LITERAL), value(v)
	{
	}

	virtual ~literal()
	{
	}

	const data_value& get_value() const
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
		col_ref = std::unique_ptr<column_ref>(cr);
	}
};



/**
 * Contains a break out of all of the query parts.
 */
class query
{
public:
	typedef std::vector<node_handle_type> select_list_type;

private:
	select_list_type select_expressions;

public:
	void select(node_list_type &s)
	{
	while (!s.empty())
		{
			select_expressions.insert(select_expressions.begin(),
					node_handle_type(s.top().release()));
			s.pop();
		}
	}

	/**
	 * Provides access to the list of select
	 * expressions.
	 */
	select_list_type& get_select_expressions()
	{
	return select_expressions;
	}

};

/**
 * Pushes a new column reference value onto the stack.
 */
struct push_column_ref: action_base<push_column_ref>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	s.push(node_handle_type(new column_ref(m)));
	}
};

/**
 * Pushes a new table reference value onto the stack.
 */
struct push_table_ref: action_base<push_table_ref>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	s.push(node_handle_type(new table_ref(m)));
	}
};

/**
 * Pushes a new literal string value onto the stack.
 */
struct push_literal_str: action_base<push_literal_str>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	data_value v;
	v.set_value(column::data_type::varchar, m);
	s.push(node_handle_type(new literal(v)));
	}
};

/**
 * Pushes a new literal numeric value onto the stack.
 */
struct push_literal_num: action_base<push_literal_num>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	data_value v;

	if (m.size() > 9)
		{
			v.set_value(column::data_type::bigint, m);
		}
	else
		{
			v.set_value(column::data_type::integer, m);
		}

	s.push(node_handle_type(new literal(v)));
	}
};

/**
 * Takes the top two items off the top of the stack, and creates
 * a binop. It then pushes the binop onto the stack.
 */
struct push_binop: action_base<push_binop>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	auto type = node::node_type::ROOT;

	switch (m[0])
		{
	case '+':
		type = node::node_type::OP_ADD;
		break;
	case '-':
		type = node::node_type::OP_SUB;
		break;
	case '*':
		type = node::node_type::OP_MUL;
		break;
	case '/':
		type = node::node_type::OP_DIV;
		break;
	case '%':
		type = node::node_type::OP_MOD;
		break;
	case '|':
		if (m[1] == '|')
			{
				type = node::node_type::OP_CAT;
			}
		break;
		}

	// Pop the two top nodes so they can be joined.
	auto left = s.top().release();
	s.pop();
	auto right = s.top().release();
	s.pop();

	// Replace the top node.
	s.push(node_handle_type(new binop(type, left, right)));
	}
};

/**
 * Takes the top item off the top of the stack, which
 * must be a column_ref, and then sets the table_ref's col_ref.
 */
struct push_deref: action_base<push_deref>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{

	auto col_ref = dynamic_cast<column_ref*>(s.top().release());
	s.pop();

	auto tbl_ref = dynamic_cast<table_ref*>(s.top().get());

	if (col_ref == nullptr)
		{
			throw std::invalid_argument("expected top of stack to be a column_ref node.");
		}

	if (tbl_ref == nullptr)
		{
			throw std::invalid_argument("expected top-1 of stack to be a table_ref node.");
		}

	tbl_ref->set_column_ref(col_ref);

	}
};


/**
 * Sweeps the stack into a list to define the list of select expressions.
 */
struct select: action_base<select>
{
	static void apply(const std::string& m, node_list_type& s, query& q)
	{
	q.select(s);
	}
};

} // namespace actions
} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PARSER_ACTIONS_H__
