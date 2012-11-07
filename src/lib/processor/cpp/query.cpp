#include <set>
#include <stdexcept>

#include <common/cpp/expected.h>
#include <processor/cpp/query.h>
#include <processor/cpp/query_parser.h>

namespace lattice {
namespace processor {

query::query(metadata& _md, const std::string& query_data) :
		md(_md)
{
	if (query_data.size() == 0)
		{
			return;
		}

	query_parser parser(md, query_data);

	parser.parse();

	// Get an ordered list of column names.
	auto column_list = parser.get_query().get_column_vector();

	auto& se_list = parser.get_query().get_select_expressions();

	// Setup the select expressions.
	for (auto& se : se_list)
		{
			auto* node = se.get();
			auto* sev = new select_expr_evaluator(md, ctx, node);
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
			void *output;
			void *args[1] = {nullptr};
			auto result = se->apply(args, &output);

			std::string* value = static_cast<std::string*>(output);
			tpl.push_back(*value);

			delete value;
		}

	return tpl;
}

} // namespace processor
} // namespace lattice

