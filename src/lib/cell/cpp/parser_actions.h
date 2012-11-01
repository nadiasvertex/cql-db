/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_CELL_PARSER_ACTIONS_H__
#define __LATTICE_CELL_PARSER_ACTIONS_H__

#include <memory>
#include <vector>

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

class node {
public:
	enum class node_type {
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
	node(node_type _type):type(_type) {}
};

typedef std::unique_ptr<node> node_handle_type;

class binop : public node {
	node_handle_type left;
	node_handle_type right;
public:
	binop(node::node_type _type, node* _left, node* _right) :
		node(_type), left(_left), right(_right) {}
};

class literal : public node {
	data_value value;
public:
	literal(const data_value& v):node(node::node_type::LITERAL),
		value(v) {}
};


} // namespace actions
} // namespace cell
} // namespace lattice


#endif // __LATTICE_CELL_PARSER_ACTIONS_H__
