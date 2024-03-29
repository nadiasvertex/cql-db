#include <cstdio>
#include <jit/jit-dump.h>

#include <processor/cpp/evaluator.h>
#include <processor/cpp/row_buffer.h>

namespace lattice {
namespace processor {

#include <processor/cpp/evaluator_row_fetch.h>

select_expr_evaluator::select_expr_evaluator(metadata& _md,
		jit_context& context, actions::node_handle_type _se, select_fields& _fields) :
		md(_md), jit_function(context), se(_se), fields(_fields)
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

	jit_dump_function(stdout, raw(), "select_expr");
}

jit_type_t select_expr_evaluator::create_signature()
{
	// Return type, followed by 1 void* (row_buffer)
	return signature_helper(jit_type_void_ptr, jit_type_void_ptr, end_params);
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

std::uint8_t select_expr_evaluator::size_in_bytes(
		const cell::column::data_type type) const
{
	switch (type)
		{
		case cell::column::data_type::smallint:
			return 2;

		case cell::column::data_type::integer:
			return 4;

		case cell::column::data_type::bigint:
			return 8;

		case cell::column::data_type::real:
			return 4;

		case cell::column::data_type::double_precision:
			return 8;

		case cell::column::data_type::varchar:
			return sizeof(void*);
		}

	throw std::invalid_argument(
			"unknown value type when constructing select expression evaluator.");
}

auto select_expr_evaluator::eval_leaf(actions::node_handle_type node) -> value_type
{
	switch (node->get_type())
		{
		case actions::node::node_type::LITERAL:
			{
				auto* l = dynamic_cast<actions::literal*>(node.get());
				if (l == nullptr)
					{
						throw std::invalid_argument(
								"node claims to be a literal value, but dynamic cast yields nullptr.");
					}
				auto& v = l->get_value();
				return std::make_tuple(literal_value_of(v), v.get_type());
			}

		case actions::node::node_type::COLUMN_REF:
			{
				auto* cr = dynamic_cast<actions::column_ref*>(node.get());
				if (cr == nullptr)
					{
						throw std::invalid_argument(
								"node claims to be a column reference, but dynamic cast yields nullptr.");
					}

				auto index = cr->get_index();
				auto& type = fields.column_types[index];

				auto jv_index = new_constant(index, jit_type_sys_int);
				auto jv_row_buffer = get_param(0);

				return gen_column_fetch(type.type, jv_row_buffer, jv_index);
			}
		break;

		case actions::node::node_type::TABLE_REF:
		break;
		}

	throw std::invalid_argument(
				"unknown leaf type in eval_leaf.");
}

auto select_expr_evaluator::gen_column_fetch(const cell::column::data_type type,
		jit_value row_buffer, jit_value column_index) -> value_type
{
	jit_value args[2];

	args[0] = row_buffer;
	args[1] = column_index;

#define FETCH(type_name, jit_typename)                                       \
		std::make_tuple(                                                       \
	insn_call_native("fetch_" #type_name "_value",                            \
			reinterpret_cast<void*>(fetch_##type_name##_value),                 \
			signature_helper(jit_typename, jit_type_void_ptr, jit_type_sys_int, \
					end_params), (_jit_value**) args, 2, 0), type                 \
	);

	switch (type)
		{
		case cell::column::data_type::smallint:
			return FETCH(int16, jit_type_short);
			/* no break */

			case cell::column::data_type::integer:
			return FETCH(int32, jit_type_int);
			/* no break */

			case cell::column::data_type::bigint:
			return FETCH(int64, jit_type_long);
			/* no break */

			case cell::column::data_type::real:
			return FETCH(float, jit_type_float32);
			/* no break */

			case cell::column::data_type::double_precision:
			return FETCH(double, jit_type_float64);
			/* no break */

			case cell::column::data_type::varchar:
			return FETCH(string, jit_type_void_ptr);
			/* no break */
		}

#undef FETCH

	throw std::invalid_argument(
			"unknown value type when constructing select expression evaluator.");
}

auto select_expr_evaluator::gen_unboxed_binop(actions::node_handle_type node,
		value_type& left, value_type& right) -> value_type
{
	auto& l = std::get<0>(left);
	auto& r = std::get<0>(right);
	auto& t = std::get<1>(left);

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

auto select_expr_evaluator::gen_string_conversion(actions::node_handle_type node,
		value_type& value) -> value_type
{
	auto type = std::get<1>(value);
	auto input = std::get<0>(value);

	jit_value args[2];

	auto t1 = value_of(type);
	jit_insn_store(raw(), t1.raw(), input.raw());

	auto t2 = insn_address_of(t1);
	args[0] = new_constant((jit_int) type, jit_type_int);
	args[1] = t2;

	auto result = insn_call_native("convert_value_to_string",
			reinterpret_cast<void*>(convert_value_to_string),
			signature_helper(jit_type_void_ptr, jit_type_int, jit_type_void_ptr,
					end_params), (_jit_value**) args, 2, 0);

	return std::make_tuple(result, cell::column::data_type::varchar);
}

auto select_expr_evaluator::eval_binop(actions::node_handle_type node) -> value_type
{
	actions::binop* op = dynamic_cast<actions::binop*>(node.get());

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

	return gen_unboxed_binop(node, lvalue, rvalue);

}

auto select_expr_evaluator::evaluate(actions::node_handle_type node) -> value_type
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
