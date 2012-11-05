/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_CELL_PARSER_ACTIONS_H__
#define __LATTICE_CELL_PARSER_ACTIONS_H__

#include <algorithm>
#include <vector>

#include <cell/cpp/predicate.h>
#include <cell/cpp/database.h>
#include <cell/cpp/parser_nodes.h>

namespace lattice
{
namespace cell
{

namespace actions
{

using namespace pegtl;

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

/** A handle to a single query. */
typedef std::unique_ptr<query> query_handle_type;

/** A stack of query handles. */
typedef std::stack<actions::query_handle_type> query_stack_type;

/**
 * Pushes a new column reference value onto the stack.
 */
struct push_column_ref: action_base<push_column_ref>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
	{
		s.push(node_handle_type(new column_ref(m)));
	}
};

/**
 * Pushes a new table reference value onto the stack.
 */
struct push_table_ref: action_base<push_table_ref>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
	{
		s.push(node_handle_type(new table_ref(m)));
	}
};

/**
 * Pushes a new literal string value onto the stack.
 */
struct push_literal_str: action_base<push_literal_str>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
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
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
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
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
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
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& q)
	{

		auto col_ref = dynamic_cast<column_ref*>(s.top().release());
		s.pop();

		auto tbl_ref = dynamic_cast<table_ref*>(s.top().get());

		if (col_ref == nullptr)
			{
				throw std::invalid_argument(
						"expected top of stack to be a column_ref node.");
			}

		if (tbl_ref == nullptr)
			{
				throw std::invalid_argument(
						"expected top-1 of stack to be a table_ref node.");
			}

		tbl_ref->set_column_ref(col_ref);

	}
};

/**
 * Sweeps the stack into a list to define the list of select expressions.
 */
struct select: action_base<select>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		qs.top()->select(s);
	}
};

} // namespace actions
} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PARSER_ACTIONS_H__
