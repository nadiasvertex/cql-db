#ifndef __LATTICE_PROCESSOR_ROW_BUFFER_H__
#define __LATTICE_PROCESSOR_ROW_BUFFER_H__

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cell/cpp/column.h>
#include <cell/cpp/data_value.h>

#include <processor/cpp/metadata.h>
#include <processor/proto/row.pb.h>

namespace lattice {
namespace processor {

class row_buffer
{
public:
	/**
	 * Contains rows gathered from cells, tightly packed
	 * into a serializable buffer.
	 */
	typedef struct
	{
		/** The id of the row. */
		std::uint64_t id;
		/** The size of the buffer. */
		std::uint64_t size;
		/** The buffer itself. */
		std::uint8_t* buffer;
	} row_type;

	/** Incoming rows are pushed on the back and
	 * rows being processed are popped off the front.
	 */
	typedef std::deque<row_type> row_queue_type;

	/**
	 * The row which is currently being processed is unpacked
	 * and stored in an object of this type for easy access.
	 */
	typedef std::vector<cell::data_value> unpacked_row_type;

	/**
	 * The column type information is stored here. This information
	 * is used in unpacking.
	 */
	typedef std::vector<cell::column> row_header_type;
private:

	/**
	 * The queue of rows to process.
	 */
	row_queue_type rows;

	/**
	 * Serializes access to the rows queue.
	 */
	std::mutex rows_lock;

	/**
	 * The row currently being processed.
	 */
	unpacked_row_type current_row;

	/**
	 * The header, which contains type information
	 * about the columns stored here.
	 */
	row_header_type header;

public:
	row_buffer(const row_header_type& rh):header(rh)
	{
	}

	/**
	 * Take a Row message and enqueue it into the
	 * buffer.
	 *
	 * @param row: The row message to enqueue.
	 *
	 * @notes: This method is thread safe.
	 */
	void enqueue(lattice::processor::Row& row)
	{
		auto data = row.data();
		auto size = data.size();

		auto buffer = new std::uint8_t[size];
		std::memcpy(buffer, data.c_str(), size);

		std::lock_guard < std::mutex > lock(rows_lock);
		rows.emplace_back(row_type
			{
			row.id(), row.data().size(), buffer
			});
	}

	/**
	 * Take the next item from the front of the queue
	 * and unpack it into the current_row vector.
	 *
	 * @notes: This method is thread safe.
	 */
	void dequeue()
	{
		// Dequeue a row from the buffer.
		row_type row;
			{
				std::lock_guard < std::mutex > lock(rows_lock);
				row = rows.front();
				rows.pop_front();
			}

		current_row.clear();

		// Unpack the row data into the current row.
		std::uint32_t index = 0;
		for (auto& h : header)
			{
				current_row.emplace_back(cell::data_value(h.type));
				index += current_row.back().read(row.buffer + index);
			}

		// Clean up the data buffer.
		delete[] row.buffer;
	}

	/**
	 * Provides a reference to the current row vector. The items in
	 * this vector correspond 1:1 with the items in the row_header
	 * w/r to type information.
	 */
	unpacked_row_type& get_current_row()
	{
		return current_row;
	}

};

} // end namespace processor
} // end namespace lattice 

#endif //__LATTICE_PROCESSOR_ROW_BUFFER_H__
