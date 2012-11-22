#ifndef __LATTICE_PROCESSOR_QUERY_ANALYZER_H__
#define __LATTICE_PROCESSOR_QUERY_ANALYZER_H__

#include <common/cpp/expected.h>
#include <processor/cpp/metadata.h>
#include <processor/cpp/select_fields.h>
#include <processor/cpp/parser_actions.h>

namespace lattice {
namespace processor {

class query_analyzer
{
   metadata& md;

   select_fields fields;

   actions::query& query;

public:
   query_analyzer(metadata& _md, actions::query& _query);

   expected<bool> check();

   select_fields& get_fields()
   {
      return fields;
   }
};


} // end namespace processor
} // end namespace lattice

#endif // __LATTICE_PROCESSOR_QUERY_ANALYZER_H__
