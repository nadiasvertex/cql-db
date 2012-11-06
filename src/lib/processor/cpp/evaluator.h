#ifndef __LATTICE_PROCESSOR_EVALUATOR_H__
#define __LATTICE_PROCESSOR_EVALUATOR_H__

#include <jit/jit-plus.h>
#include <processor/cpp/query_parser.h>

namespace lattice {
namespace processor {

class select_expr_evaluator: public jit_function
{
	metadata& md;
	actions::node* se;

public:
	select_expr_evaluator(metadata& _md, jit_context& context,
			actions::node* _se);
	/**
	 * Build the code to evaluate this select expression.
	 */
	virtual void build();

protected:
	virtual jit_type_t create_signature();

	/**
	 * Provides a jit_value object for a literal value.
	 *
	 * @param o: The data object.
	 */
	jit_value literal_value_of(const cell::data_value& o);

	/**
	 * Gets a new value of the given type.
	 *
	 * @param type: The data type.
	 */
	jit_value value_of(const cell::column::data_type type);

	/**
	 * Evaluates a leaf node, and provides a jit value that
	 * represents the evaluation of that leaf node.
	 *
	 * @param node: The leaf node to evaluate.
	 */
	jit_value eval_leaf(actions::node* node);

	/**
	 * Generates a constant binary operation. We assume that the values
	 * are simple constant values that don't require fetching.
	 *
	 * @param node: The node to generate a binop for.
	 * @param left: The left input value.
	 * @param right: The right input value.
	 *
	 * @return: A new value that represents the output of the
	 *          instruction.
	 */
	jit_value gen_constant_binop(actions::node* node, jit_value& left,
			jit_value& right);

	/**
	 * Evaluates a binary operation.
	 *
	 * @param node: The node to evaluate.
	 *
	 * @returns: A new value that represents the output of the
	 *           instruction.
	 */
	jit_value eval_binop(actions::node* node);

	/**
	 * Top level evaluator. Evaluates this node, returning a single jit value
	 * as the result.
	 *
	 * @param node: The node to evaluate.
	 *
	 * @returns: A new value that represents the output of the
	 *           instruction.
	 */
	jit_value evaluate(actions::node* node);
};

} // end namespace processor
} // end namespace lattice

#endif //__LATTICE_PROCESSOR_EVALUATOR_H__
