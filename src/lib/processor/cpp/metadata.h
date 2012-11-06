#ifndef __LATTICE_PROCESSOR_METADATA_H__
#define __LATTICE_PROCESSOR_METADATA_H__

/**
 * This object tracks meta data across the lattice. Each node maintains this
 * index, and the index is shared across all query processors running in the
 * local group.
 *
 * This means that, contrary to much of the code, multiple threads may be
 * involved, so thread safety is important when updating.
 *
 */

#include <unordered_map>
#include <unordered_set>

#include <cell/cpp/column.h>

namespace lattice {
namespace processor {

class metadata
{
public:
	struct table
	{
		/** The name of the table. */
		std::string name;

		/** The map of column names to column definitions. */
		std::unordered_map<std::string, cell::column> columns;
	};

public:
	/** The type of node ids. */
	typedef unsigned long long node_id_type;

	/** Maps oids to tables. */
	typedef std::unordered_map<std::string, node_id_type> table_node_map_type;

	/** Maps table names to tables. */
	typedef std::unordered_map<std::string, table> table_map_type;

private:
	/**
	 * The map of table names to node ids.
	 */
	table_map_type table_nodes;

	/**
	 * The map of table names to table metadata.
	 */
	table_map_type tables;


public:
	metadata()
	{
	}

	/**
	 * Create a table.
	 *
	 * @param name: The name of the table.
	 * @param columns: The column definitions for the table.
	 *
	 * @returns: true if it worked, false otherwise.
	 */
	bool create_table(std::string name, std::vector<cell::column> columns)
	{
		table t;

		for (auto i = 0; i < columns.size(); ++i)
			{
				auto& col = columns[i];
				t.columns[col.name] = col;
			}

		return true;
	}

};

} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_METADATA_H__
