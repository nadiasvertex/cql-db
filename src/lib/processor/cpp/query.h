#ifndef __LATTICE_PROCESSOR_QUERY_H__
#define __LATTICE_PROCESSOR_QUERY_H__

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <processor/cpp/metadata.h>
#include <processor/cpp/evaluator.h>
#include <processor/cpp/row_buffer.h>
#include <processor/cpp/query_analyzer.h>

#include <jit/jit-plus.h>

namespace lattice
{
namespace processor
{
 
typedef std::unique_ptr<select_expr_evaluator> select_evaluator_type;
typedef std::vector<select_evaluator_type> select_evaluator_list_type;

class query
{
public:

	/** Contains the output of a select. */
	typedef std::vector<std::string> tuple_type;

private:
	/**
	 * Reference to the metadata.
	 */
	metadata& md;

	/**
	 * The jit context for processing queries.
	 */
	jit_context ctx;

	/**
	 * A list of select expression evaluator objects. They have the
	 * job of actually executing each select expression and solving
	 * them.
	 */
	select_evaluator_list_type select_exprs;

	/**
	 * The query analyzer performs syntax checking, type inference,
	 * and other planning actions.
	 */
	query_analyzer* qa;

	/**
	 * Solves the query once.
	 *
	 * @returns: A tuple containing the output, if any.
	 */
	tuple_type solve_once(row_buffer& rb);

public:
	query(metadata& _md, const std::string& query_data);

	~query();


	/**
	 * Fetches a single row.
	 *
	 * @param rb: The row buffer to use.
	 *
	 * @returns: A tuple of stringified results.
	 */
	tuple_type fetch_one(row_buffer& rb)
	{
		return solve_once(rb);
	}
};

} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_QUERY_H__
