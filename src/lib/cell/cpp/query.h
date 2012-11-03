#ifndef __LATTICE_CELL_QUERY_H__
#define __LATTICE_CELL_QUERY_H__

#include <vector>

#include <cell/cpp/predicate.h>
#include <cell/cpp/database.h>
#include <cell/cpp/query_parser.h>

namespace lattice
{
namespace cell
{

class query
{
public:
	/**
	 * Binds a cursor to a predicate.
	 */
	typedef struct
	{
		/**
		 * The predicate to execute.
		 */
		predicate_handle_type predicate;

		/**
		 * The cursor to execute against.
		 */
		page_cursor& cursor;

	} predicate_group_type;

	/**
	 * Contains a list of predicates.
	 */
	typedef std::vector<predicate_group_type> predicate_group_list_type;

	typedef struct
	{
		/**
		 * The select expression
		 */
		actions::node *se;

		/**
		 * The cursor to execute against.
		 */
		page_cursor* cursor;
	} select_group_type;

	typedef std::vector<select_group_type> select_group_list_type;

	/** Contains the output of a select. */
	typedef std::vector<std::string> tuple_type;

private:
	database& db;

	/**
	 * The parser, also stores the AST for traversal
	 * by the expression solver.
	 */
	query_parser parser;

	/** A list of select expressions */
	select_group_list_type select_exprs;

	/** A list of predicates. */
	predicate_group_list_type predicates;

	/**
	 * Evaluate the select expression.
	 *
	 * @param sg: The select expression group to evaluate.
	 *
	 * @returns: A single data value, which may be of null type.
	 */
	data_value dispatch(select_group_type& sg);

	/**
	 * Solves the query once.
	 *
	 * @returns: A tuple containing the output, if any.
	 */
	tuple_type solve_once();

public:
	query(database& _db, const std::string& query_data) :
			db(_db), parser(_db, query_data)
	{
	if (query_data.size() == 0)
		{
			return;
		}

	parser.parse();

	auto& se_list = parser.get_query().get_select_expressions();

	// Setup the select expressions.
	for (auto& se : se_list)
		{
			auto* node = se.get();
			select_exprs.push_back(select_group_type{node, nullptr});
		}
	}

			tuple_type fetch_one()
			{
				return solve_once();
			}


};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_QUERY_H__
