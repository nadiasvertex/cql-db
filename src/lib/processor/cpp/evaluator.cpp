#include <processor/cpp/evaluator.h>

namespace lattice {
namespace processor {

select_expr_evaluator::select_expr_evaluator(metadata& _md,
		jit_context& context, actions::node* _se) :
		md(_md), jit_function(context), se(_se)
{
	create();
	set_recompilable();
}

void select_expr_evaluator::build()
{
	auto results = evaluate(se);
	insn_return(results);
}

jit_type_t select_expr_evaluator::create_signature()
{
	// Return type, followed by no parameters.
	return signature_helper(jit_type_void_ptr, end_params);
}

jit_value select_expr_evaluator::literal_value_of(const cell::data_value& o)
{
	switch (o.get_type())
		{
		case cell::column::data_type::smallint:
			return new_constant(o.value.i16, jit_type_short);

		case cell::column::data_type::integer:
			return new_constant(o.value.i32, jit_type_int);

		case cell::column::data_type::bigint:
			return new_constant(o.value.i64, jit_type_long);

		case cell::column::data_type::real:
			return new_constant(o.value.f32, jit_type_float32);

		case cell::column::data_type::double_precision:
			return new_constant(o.value.f64, jit_type_float64);

		case cell::column::data_type::varchar:
			return new_constant(o.value.s, jit_type_void_ptr);
		}

	throw std::invalid_argument(
			"unknown literal value type when constructing select expression evaluator.");
}

jit_value select_expr_evaluator::value_of(const cell::column::data_type type)
{
	switch (type)
		{
		case cell::column::data_type::smallint:
			return new_value(jit_type_short);

		case cell::column::data_type::integer:
			return new_value(jit_type_int);

		case cell::column::data_type::bigint:
			return new_value(jit_type_long);

		case cell::column::data_type::real:
			return new_value(jit_type_float32);

		case cell::column::data_type::double_precision:
			return new_value(jit_type_float64);

		case cell::column::data_type::varchar:
			return new_value(jit_type_void_ptr);
		}

	throw std::invalid_argument(
			"unknown value type when constructing select expression evaluator.");
}

/**
 * Evaluates a leaf node, and provides a jit value that
 * represents the evaluation of that leaf node.
 *
 * @param node: The leaf node to evaluate.
 */
jit_value select_expr_evaluator::eval_leaf(actions::node* node)
{
	switch (node->get_type())
		{
		case actions::node::node_type::LITERAL:
			{
				auto* l = dynamic_cast<actions::literal*>(node);
				if (l == nullptr)
					{
						throw std::invalid_argument(
								"node claims to be a literal value, but dynamic cast yields nullptr.");
					}
				return literal_value_of(l->get_value());
			}

		case actions::node::node_type::COLUMN_REF:
		case actions::node::node_type::TABLE_REF:
		break;
		}
}

jit_value select_expr_evaluator::gen_constant_binop(actions::node* node,
		jit_value& left, jit_value& right)
{
	switch (node->get_type())
		{
		case actions::node::node_type::OP_ADD:
			return left + right;
		case actions::node::node_type::OP_SUB:
			return left - right;
		case actions::node::node_type::OP_MUL:
			return left * right;
		case actions::node::node_type::OP_DIV:
			return left / right;
		}
}

jit_value select_expr_evaluator::eval_binop(actions::node* node)
{
	actions::binop* op = dynamic_cast<actions::binop*>(node);

	// Op is apparently not really a binop.
	if (op == nullptr)
		{
			throw std::invalid_argument(
					"evaluating a node that claims to be a binop, but dynamic cast yields nullptr.");
		}

	auto left_node = op->get_left();
	auto right_node = op->get_right();

	auto lvalue = evaluate(left_node);
	auto rvalue = evaluate(right_node);

	if (lvalue.is_constant() && rvalue.is_constant())
		{
			return gen_constant_binop(node, lvalue, rvalue);
		}
}

jit_value select_expr_evaluator::evaluate(actions::node* node)
{
	switch (node->get_type())
		{
		case actions::node::node_type::OP_ADD:
		case actions::node::node_type::OP_SUB:
		case actions::node::node_type::OP_MUL:
		case actions::node::node_type::OP_DIV:
			return eval_binop(node);

		case actions::node::node_type::LITERAL:
		case actions::node::node_type::COLUMN_REF:
		case actions::node::node_type::TABLE_REF:
			return eval_leaf(node);

		default:
			throw std::invalid_argument(
					"unknown operation requested in select expression evaluator.");
		}
}

} // namespace processor
} // namespace lattice
