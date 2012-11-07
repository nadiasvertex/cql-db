/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_PROCESSOR_PARSER_ACTIONS_H__
#define __LATTICE_PROCESSOR_PARSER_ACTIONS_H__

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include <processor/cpp/parser_nodes.h>

namespace lattice {
namespace processor {
namespace actions {

using namespace pegtl;

/**
 * Encapsulates a table expression. This may contain many joins.
 */
class table_expr
{
public:
	enum class join_type
	{
		LEFT_OUTER,
		RIGHT_OUTER,
		INNER,
		CROSS,
		NATURAL
	};

	typedef std::unique_ptr<table_expr> handle;

	/**
	 * Models a table join.
	 */
	class join
	{
		/**
		 * The type of join.
		 */
		join_type jt;

		/**
		 * The table expression involved in the join.
		 */
		handle tbl_expr;
	public:
		join(join_type _jt) :
				jt(_jt)
		{
		}
		;

		/**
		 * Sets the table expression for this join.
		 *
		 * @param te: The table expression.
		 */
		void set_table_expr(table_expr *te)
		{
			tbl_expr = handle(te);
		}
	};

	/** Type for storing a list of joins. */
	typedef std::vector<join> join_list_type;

private:
	/**
	 * A list of joins involved in this table. May be
	 * empty.
	 */
	join_list_type joins;

	/**
	 * The name of the table involved in the table expression
	 * (if any.)
	 */
	std::string name;

public:
	/**
	 * Sets the name of the table involved in this table
	 * expression.
	 *
	 * @param n: The name of the table.
	 */
	void set_table_name(const std::string& n)
	{
		name = n;
	}

	/**
	 * Sets the name of the table in the last join added
	 * to the table expression.
	 *
	 * @param te: The table expression for the current join.
	 */
	void set_join_table_expr(table_expr *te)
	{
		joins.back().set_table_expr(te);
	}

	/**
	 * Adds a new join to the table expression.
	 *
	 * @param jt: The type of join to add.
	 */
	void add_join(join_type jt)
	{
		joins.emplace_back(jt);
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
	/**
	 * The list of select expressions for this query.
	 */
	select_list_type select_expressions;

	/**
	 * Provides a mapping between column names and column indexes. Columns
	 * are mapped to indexes based on their first mention.
	 */
	std::unordered_map<std::string, int> column_map;

public:
	/** The table expression for this query. */
	table_expr table_expression;

	/**
	 * Sweeps all nodes on the given stack into
	 * select expressions for this query.
	 *
	 * @param s: The node stack to process.
	 */
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

	/**
	 * Finds the index of the column. If the column
	 * doesn't have an index associated, a new one is
	 * assigned.
	 *
	 * @param name: The name of the column.
	 *
	 * @returns: The column's index in the row.
	 */
	int get_column_index(const std::string& name)
	{
		auto pos = column_map.find(name);
		if (pos==column_map.end())
			{
				pos = column_map.insert(std::make_pair(name, column_map.size())).first;
			}

		return pos->second;
	}

	/**
	 * Provides a list of ordered column names, in the order they appear in
	 * the query.
	 */
	std::vector<std::string> get_column_vector()
		{
			std::map<int, std::string> names;
			for(auto& n : column_map)
				{
					names[n.second] = n.first;
				}

			std::vector<std::string> col_list;
			for(auto& p : names)
				{
					col_list.push_back(p.second);
				}

			return col_list;
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
			query_stack_type& qs)
	{
		auto& q = qs.top();
		s.push(node_handle_type(new column_ref(m, q->get_column_index(m))));
	}
};

/**
 * Pushes a new table reference value onto the stack.
 */
struct push_table_ref: action_base<push_table_ref>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		s.push(node_handle_type(new table_ref(m)));
	}
};

/**
 * Takes the input string and converts it to two nodes: a table_ref and a
 * column_ref. The column_ref is plugged into the table_ref, which is
 * left on the stack.
 */
struct push_deref: action_base<push_deref>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		auto p = m.find('.');

		auto table_name = m.substr(0, p);
		auto column_name = m.substr(p + 1, m.size() - p);


		auto& q = qs.top();
		auto* tbl_ref = new table_ref(table_name);
		auto* col_ref = new column_ref(column_name, q->get_column_index(m));

		tbl_ref->set_column_ref(col_ref);

		s.push(node_handle_type(tbl_ref));
	}
};

/**
 * Adds a new table join.
 */
template<table_expr::join_type JT>
struct add_table_join: action_base<add_table_join<JT>>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		auto* q = qs.top().get();
		q->table_expression.add_join(JT);
	}
};

/**
 * Sets the name of the current table expression.
 */
struct set_table_name: action_base<set_table_name>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		auto* q = qs.top().get();
		q->table_expression.set_table_name(m);
	}
};

/**
 * Pushes a new literal string value onto the stack.
 */
struct push_literal_str: action_base<push_literal_str>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		cell::data_value v;
		v.set_value(cell::column::data_type::varchar, m);
		s.push(node_handle_type(new literal(v)));
	}
};

/**
 * Pushes a new literal numeric value onto the stack.
 */
struct push_literal_num: action_base<push_literal_num>
{
	static void apply(const std::string& m, node_list_type& s,
			query_stack_type& qs)
	{
		cell::data_value v;

		if (m.size() > 9)
			{
				v.set_value(cell::column::data_type::bigint, m);
			}
		else
			{
				v.set_value(cell::column::data_type::integer, m);
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
		auto right = s.top().release();
		s.pop();
		auto left = s.top().release();
		s.pop();

		// Replace the top node.
		s.push(node_handle_type(new binop(type, left, right)));
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
} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_PARSER_ACTIONS_H__
