#include <sstream>
#include <cell/cpp/command_processor.h>
#include <cell/cpp/data_value.h>

namespace lattice {
namespace cell {

page::object_id_type command_processor::create_transaction()
{
	auto txn_id = ++next_transaction_id;
	transactions.insert(std::make_pair(txn_id, transaction()));

	return txn_id;
}

page::object_id_type command_processor::create_cursor(
		page::object_id_type txn_id, page::object_id_type table_id)
{
	auto pos = transactions.find(txn_id);
	if (pos == transactions.end())
		{
			return 0;
		}

	// Get transaction
	auto& txn = pos->second;

	// Get the table
   auto t = db.get_table(table_id);

	// Create a cursor on the table.
	return txn.create_cursor(t);
}

std::string command_processor::fetch_columns(page::object_id_type txn_id,
		page::object_id_type cursor_id, std::vector<int> column_indexes)
{
	auto pos = transactions.find(txn_id);
	if (pos == transactions.end())
		{
			return std::string();
		}

	// Get transaction
	auto& txn = pos->second;

	// Get cursor
	auto& cursor = txn.get_cursor(cursor_id);
	auto& row = cursor.it;
	auto& t   = cursor.t;

	// Temporary buffer for writing out data.
	std::stringstream out;
	std::vector<bool> present;

	present.assign(t->get_number_of_columns(), false);
	for(auto index : column_indexes)
		{
			present[index] = true;
		}

	t->fetch_row(row, present, out);
	return out.str();
}

} // namespace cell
} // namespace lattice

