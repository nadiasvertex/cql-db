#include <set>
#include <stdexcept>

#include <common/cpp/expected.h>
#include <processor/cpp/query.h>
#include <processor/cpp/evaluator.h>

namespace lattice {
namespace processor {

query::query(metadata& _md, const std::string& query_data) :
		md(_md), parser(_md, query_data)
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
			auto result = se->apply(&output, nullptr);
			//tpl.push_back(result.to_string());
		}

	return tpl;
}

} // namespace processor
} // namespace lattice

