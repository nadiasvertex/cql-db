#include <set>
#include <sstream>
#include <stdexcept>

#include <common/cpp/expected.h>
#include <processor/cpp/query.h>
#include <processor/cpp/query_parser.h>

namespace lattice {
namespace processor {

/**
 * Does syntax checking and column/table binding. This
 * must be run before executing fetch_XXXX.
 *
 * @returns: A tuple of (error message, passed).
 *
 * If passed is true, then error message is empty. Otherwise it contains
 * the reason that the check failed.
 */
static query::check_type check(metadata& md, query_parser& parser,
		query::column_index_type_map_type& column_types)
{
	auto& query = parser.get_query();

	// Get an ordered list of column names.
	auto column_list = query.get_column_vector();

	// Get the table expression
	auto& table = query.get_table_expression();

	int index = -1;

	auto missing_column_msg =
			[](const std::string& table_name, const std::string& column_name)
				{
					std::stringstream msg;
					msg << "There is no column named '" << column_name
					<< "' in table '" << table_name
					<< "'";

					return std::make_tuple(msg.str(), false);
				};

	// Resolve the column names to actual tables.
	for (auto& col : column_list)
		{
			++index;

			auto pos = col.find('.');

			// If the column name starts with a "." then the column is not
			// table qualified, and we need to look it up in the table
			// expression's base table name.
			if (pos == std::string::npos)
				{
					// Find the base table in the metadata, then make sure that
					// the column name is valid. Collect the type information
					// into a map for later use.
					auto info = md.get_column_type(table.get_table_name(), col);

					if (std::get<1>(info) == false)
						{
							return missing_column_msg(table.get_table_name(), col);
						}

					// Map the type to the column index.
					column_types.insert(std::make_pair(index, std::get<0>(info)));
				}
			else
				{
					auto table_name = col.substr(0, pos);
					auto column_name = col.substr(pos + 1, col.size() - pos);

					// Find the specified table in the metadata, and then
					// make sure that the column exists. If so, get the type.
					auto info = md.get_column_type(table_name, column_name);

					if (std::get<1>(info) == false)
						{
							return missing_column_msg(table_name, column_name);
						}

					// Map the type to the column index.
					column_types.insert(std::make_pair(index, std::get<0>(info)));
				}
		}

}

query::query(metadata& _md, const std::string& query_data) :
		md(_md)
{
	if (query_data.size() == 0)
		{
			return;
		}

	query::column_index_type_map_type column_types;

	query_parser parser(md, query_data);

	parser.parse();

	auto& se_list = parser.get_query().get_select_expressions();

	//auto check_results = check(md, parser, column_types);

	// Setup the select expressions.
	for (auto& se : se_list)
		{
			auto* sev = new select_expr_evaluator(md, ctx, se);
			select_exprs.push_back(select_evaluator_type(sev));
		}
}

query::tuple_type query::solve_once()
{
	tuple_type tpl;

	// 1. Execute predicates

	// 2. Solve selects

	for (auto& se : select_exprs)
		{
			uint64_t row_buffer=0;

			void *output;
			void *args[1] =
				{
				&row_buffer
				};
			auto result = se->apply(args, &output);

			std::string* value = static_cast<std::string*>(output);

			if (value!=nullptr)
				{
					tpl.push_back(*value);
					delete value;
				}
		}

	return tpl;
}

} // namespace processor
} // namespace lattice

