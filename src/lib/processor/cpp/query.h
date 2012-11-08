#ifndef __LATTICE_PROCESSOR_QUERY_H__
#define __LATTICE_PROCESSOR_QUERY_H__

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <processor/cpp/metadata.h>
#include <processor/cpp/evaluator.h>


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

	/** Contains the output of the syntax check. */
	typedef std::tuple<std::string, bool> check_type;

	/** Contains a mapping between a column index and the type of the column. */
	typedef std::unordered_map<int, cell::column> column_index_type_map_type;

private:
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
	 * Solves the query once.
	 *
	 * @returns: A tuple containing the output, if any.
	 */
	tuple_type solve_once();

public:
	query(metadata& _md, const std::string& query_data);

	tuple_type fetch_one()
	{
		return solve_once();
	}



};

} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_QUERY_H__
