#include <set>
#include <sstream>
#include <stdexcept>

#include <common/cpp/expected.h>
#include <processor/cpp/parser_exceptions.h>
#include <processor/cpp/query.h>
#include <processor/cpp/query_parser.h>
#include <processor/cpp/select_fields.h>

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
static expected<bool> check(metadata& md, query_parser& parser,
		select_fields& fields)
{
	auto& query = parser.get_query();

	// Get an ordered list of column names.
	fields.column_names = query.get_column_vector();
	auto& column_list = fields.column_names;

	// Get the table expression
	auto& table = query.get_table_expression();

	int index = -1;

	// De-duplication of missing column error message.
	auto missing_column_msg =
			[](const std::string& table_name, const std::string& column_name)
				{
					return expected<bool>::from_exception(
							unknown_column_error(table_name, column_name));
				};

	auto& base_table = table.get_table_name();

	// Make sure the specified base table name exists.
	if (base_table.size() > 0 && !md.has_table(base_table))
		{
			return expected<bool>::from_exception(unknown_table_error(base_table));
		}

	// If a column list was specified, make sure that a 'from' clause exists.
	if (column_list.size() > 0 && base_table.size() == 0)
		{
			return expected<bool>::from_exception(missing_from());
		}

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
					auto info = md.get_column_type(base_table, col);

					if (std::get<1>(info) == false)
						{
							return missing_column_msg(base_table, col);
						}

					// Map the type to the column index.
					fields.column_types.insert(std::make_pair(index, std::get<0>(info)));
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
					fields.column_types.insert(std::make_pair(index, std::get<0>(info)));
				}
		}

	return expected<bool>(true);
}

query::query(metadata& _md, const std::string& query_data) :
		md(_md)
{
	if (query_data.size() == 0)
		{
			return;
		}

	auto fields = select_fields_handle(new select_fields);

	query_parser parser(md, query_data);

	parser.parse();

	auto& se_list = parser.get_query().get_select_expressions();

	auto check_results = check(md, parser, *(fields.get()));

	// Setup the select expressions.
	for (auto& se : se_list)
		{
			auto* sev = new select_expr_evaluator(md, ctx, se, fields);
			if (sev != nullptr)
				{
					select_exprs.push_back(select_evaluator_type(sev));
				}
		}
}

query::tuple_type query::solve_once()
{
	tuple_type tpl;

	// 1. Execute predicates

	// 2. Solve selects

	for (auto& se : select_exprs)
		{
			uint64_t row_buffer = 0;

			void *output;
			void *args[1] =
				{
				&row_buffer
				};
			auto result = se->apply(args, &output);

			std::string* value = static_cast<std::string*>(output);

			if (value != nullptr)
				{
					tpl.push_back(*value);
					delete value;
				}
		}

	return tpl;
}

} // namespace processor
} // namespace lattice

