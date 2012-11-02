/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_CELL_PARSER_ACTIONS_H__
#define __LATTICE_CELL_PARSER_ACTIONS_H__

#include <memory>
#include <stack>

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
		LITERAL
	};

private:
	node_type type;

public:
	node(node_type _type) :
			type(_type)
	{
	}
};

typedef std::unique_ptr<node> node_handle_type;

typedef std::stack<node_handle_type> node_list_type;

class binop: public node
{
	node_handle_type left;
	node_handle_type right;
public:
	binop(node::node_type _type, node* _left, node* _right) :
			node(_type), left(_left), right(_right)
	{
	}
};

class literal: public node
{
	data_value value;
public:
	literal(const data_value& v) :
			node(node::node_type::LITERAL), value(v)
	{
	}
};

struct push_binop: action_base<push_binop>
{
	static void apply(const std::string& m, node_list_type& s)
	{
	node::node_type type;

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

	auto left = s.top().release();
	s.pop();
	auto right = s.top().release();
	s.pop();

	auto new_node = new binop(type, left, right);
	}
};

} // namespace actions
} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PARSER_ACTIONS_H__
