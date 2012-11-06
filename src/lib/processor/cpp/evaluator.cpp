#include <processor/cpp/evaluator.h>

namespace lattice {
namespace processor {

static std::string*
convert_value_to_string(cell::column::data_type type, void* value)
{
	cell::data_value dv;

	switch (type)
		{
		case cell::column::data_type::smallint: {
			auto* i16 = static_cast<std::int16_t*>(value);
			dv.set_value(type, *i16);
			return new std::string(dv.to_string());
		}

		case cell::column::data_type::integer: {
			auto* i32 = static_cast<std::int32_t*>(value);
			dv.set_value(type, *i32);
			return new std::string(dv.to_string());
		}

		case cell::column::data_type::bigint: {
			auto* i64 = static_cast<std::int64_t*>(value);
			dv.set_value(type, *i64);
			return new std::string(dv.to_string());
		}

		case cell::column::data_type::real: {
			auto* f32 = static_cast<float*>(value);
			dv.set_value(type, *f32);
			return new std::string(dv.to_string());
		}

		case cell::column::data_type::double_precision: {
			auto* f64 = static_cast<float*>(value);
			dv.set_value(type, *f64);
			return new std::string(dv.to_string());
		}

		case cell::column::data_type::varchar:
			return new std::string(*static_cast<std::string*>(value));
		}

	throw std::invalid_argument(
			"unknown value type when converting value to string.");

}

select_expr_evaluator::select_expr_evaluator(metadata& _md,
		jit_context& context, actions::node* _se) :
		md(_md), jit_function(context), se(_se)
{
	create();
	set_recompilable();
}

void select_expr_evaluator::build()
{
	// Evaluate the select expression.
	auto temp = evaluate(se);
	// Turn the result into a string
	auto results = gen_string_conversion(se, temp);
	// Return the string.
	insn_return(std::get<0>(results));
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
auto select_expr_evaluator::eval_leaf(actions::node* node) -> value_type
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
				auto& v = l->get_value();
				return std::make_tuple(literal_value_of(v), v.get_type());
			}

		case actions::node::node_type::COLUMN_REF:
		case actions::node::node_type::TABLE_REF:
		break;
		}
}

auto select_expr_evaluator::gen_constant_binop(actions::node* node,
		value_type& left, value_type& right) -> value_type
{
	auto& l = std::get < 0 > (left);
	auto& r = std::get < 0 > (right);
	auto& t = std::get < 1 > (left);

	switch (node->get_type())
		{
		case actions::node::node_type::OP_ADD:
			return std::make_tuple(l + r, t);
		case actions::node::node_type::OP_SUB:
			return std::make_tuple(l - r, t);
		case actions::node::node_type::OP_MUL:
			return std::make_tuple(l * r, t);
		case actions::node::node_type::OP_DIV:
			return std::make_tuple(l / r, t);
		}
}

auto select_expr_evaluator::gen_string_conversion(actions::node* node,
		value_type& value) -> value_type
{
	auto type = std::get < 1 > (value);
	auto input = std::get < 0 > (value);

	jit_value args[] =
		{
		new_constant((jit_int)type, jit_type_int),
		insn_address_of(input)
		};

	auto result = insn_call_native("convert_value_to_string",
			reinterpret_cast<void*>(convert_value_to_string),
			signature_helper(jit_type_void_ptr, jit_type_int, jit_type_void_ptr,
					end_params), (_jit_value**)args, 2, 0);

	return std::make_tuple(result, cell::column::data_type::varchar);
}

auto select_expr_evaluator::eval_binop(actions::node* node) -> value_type
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

	if (std::get < 0 > (lvalue).is_constant()
			&& std::get < 0 > (rvalue).is_constant())
		{
			return gen_constant_binop(node, lvalue, rvalue);
		}
}

auto select_expr_evaluator::evaluate(actions::node* node) -> value_type
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
