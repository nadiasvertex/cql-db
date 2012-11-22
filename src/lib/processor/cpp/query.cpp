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

query::query(metadata& _md, const std::string& query_data) :
      md(_md)//, qa(nullptr)
{
   if (query_data.size() == 0)
      {
         return;
      }

   query_parser parser(md, query_data);

   parser.parse();

   //qa = new query_analyzer(md, parser.get_query());

   auto& se_list = parser.get_query().get_select_expressions();

   //auto check_results = qa->check();

   // Setup the select expressions.
   for (auto& se : se_list)
      {
         /*auto* sev = new select_expr_evaluator(md, ctx, se, qa->get_fields());
         if (sev != nullptr)
            {
               select_exprs.push_back(select_evaluator_type(sev));
            }*/
      }
}

query::~query()
{
   /*if (qa != nullptr)
      {
         delete qa;
      }*/
}

query::tuple_type query::solve_once(row_buffer& rb)
{
   tuple_type tpl;

   // 1. Execute predicates

   // 2. Solve selects
   void *row_buffer_address = static_cast<void*>(&rb);
   void *args[1] =
      {
      &row_buffer_address
      };

   for (auto& se : select_exprs)
      {
         void *output;
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

