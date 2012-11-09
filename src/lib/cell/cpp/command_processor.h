#ifndef __LATTICE_CELL_COMMAND_PROCESSOR_H__
#define __LATTICE_CELL_COMMAND_PROCESSOR_H__

#include <cell/cpp/database.h>
#include <cell/cpp/transaction.h>
#include <processor/proto/row.pb.h>
#include <cell/proto/commands.pb.h>

namespace lattice {
namespace cell {

/**
 * The command processor receives commands from the query processor or other
 * components and executes them in this cell. The command processor actually
 * binds together a cell and is the way in which data gets into and out
 * of a cell.
 *
 */
class command_processor
{
	typedef std::unordered_map<page::object_id_type, transaction> txn_map_type;

private:
	/** The one and only database object in the command processor. There
	 * is one command processor per database. */
	database db;

	/**
	 * Contains a map of open and outstanding transactions.
	 */
	txn_map_type transactions;

	/**
	 * Contains the next transaction id that will be provided.
	 */
	page::object_id_type next_transaction_id;

private:
	CommandResponse prepare(const CommandRequest& request);

public:
	command_processor()
	{
	}
	;

	/**
	 * Create a table.
	 *
	 * @param name: The name of the table.
	 * @param columns: The column definitions for the table.
	 *
	 * @returns: true if it worked, false otherwise.
	 */
	bool create_table(const std::string& name, std::vector<column*> columns)
	{
		return db.create_table(name, columns);
	}

	/**
	 * Creates a new transaction.
	 *
	 * @returns: A new transaction id. This transaction id is local to the
	 * cell, and needs to be used when corresponding with this cell.
	 */
	page::object_id_type create_transaction();

	/**
	 * Creates a new cursor.
	 *
	 * @param txn_id: The id of transaction to use.
	 * @param table_id: The id of the table to create the cursor for.
	 *
	 * @returns: A new cursor id. This cursor id is local to the
	 * transaction in this cell, and needs to be used when performing fetches.
	 */
	page::object_id_type create_cursor(page::object_id_type txn_id,
			page::object_id_type table_id);

	/**
	 * Fetches a list of columns.
	 *
	 * @param txn_id: The transaction id being used.
	 * @param cursor_id: The cursor to fetch columns from.
	 * @param column_indexes: The list of column indexes to return. They will
	 *                        be returned in the order specified.
	 */
	std::string fetch_columns(page::object_id_type txn_id,
			page::object_id_type cursor_id, std::vector<int> column_indexes);

	/**
	 * Process the command request and provide an equivalent
	 * command response.
	 *
	 * @param request: The command request to process.
	 *
	 * @returns: A new command response.
	 **/
	CommandResponse process(const CommandRequest& request);

};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_COMMAND_PROCESSOR_H__
