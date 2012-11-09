#ifndef __LATTICE_PROCESSOR_SELECT_FIELDS_H__
#define __LATTICE_PROCESSOR_SELECT_FIELDS_H__

#include <memory>
#include <unordered_map>

#include <cell/cpp/column.h>

namespace lattice {
namespace processor {

/**
 * Contains a list of the field information involved in a select expression. For
 * example, a query like SELECT c1,c2+c4,c3 FROM t1 actually references 4
 * columns, even though there are only 3 expressions.
 *
 * When fetching this data from cells, we need to specify each field: c1, c2,
 * c3, and c4. We also specify an order: c1, c2, c4, c3. This allows us to
 * bind the select expressions to a column index instead of a string, which
 * improves the query performance.
 *
 * During compilation of the query, we need to map the names and columns
 * and then provide that mapping to the various expression builders. This
 * structure provides those mappings.
 */
struct select_fields
{
	/** Contains a mapping between a column index and the type of the column. */
	typedef std::unordered_map<int, cell::column> column_index_type_map_type;

	/** Actually contains the column mapping. */
	column_index_type_map_type column_types;

	/** Contains the list of referenced column names, in the
	 * order in which they were referenced in the query.
	 */
	std::vector<std::string> column_names;
};

/** Defines a type that points to the common select fields. */
typedef std::shared_ptr<select_fields> select_fields_handle;


} // end namespace processor
} // end namespace lattice

#endif //__LATTICE_PROCESSOR_SELECT_FIELDS_H__
