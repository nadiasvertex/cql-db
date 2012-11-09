#ifndef __LATTICE_CELL_PAGE_H__
#define __LATTICE_CELL_PAGE_H__

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cell/cpp/page.h>
#include <cell/cpp/column.h>

namespace lattice {
namespace cell {

class page_cursor;

/**
 * A page contains data for a single column.
 */
class page
{
public:
	/** The type of bytes for a page. */
	typedef unsigned char byte_type;

	/** The type of object identifiers. */
	typedef unsigned long long object_id_type;

	/** The type for parameters indicating size. */
	typedef size_t size_type;

	typedef uint8_t reference_count_type;

	/** The type storing how large an atom can be. */
	typedef unsigned long atom_size_type;

	/** The type for object sets. */
	typedef std::set<object_id_type> object_set_type;

	/** The type for atom data bulk storage. */
	typedef std::stringstream atom_data_type;

	/** The type for an object reference. */
	typedef struct object_reference
	{
		bool forwarded;

		reference_count_type ref_count;

		union
		{
			// If not forwarded, this represents the actual data pointer
			std::streampos offset;

			// If forwarded, the id is set to the object id that points to the data.
			object_id_type id;
		};

		/** Create a new object reference to an offset in the page. */
		object_reference(const std::streampos& _offset) :
				forwarded(false), ref_count(1), offset(_offset)
		{
		}
		;

		/** Create a new object reference that forwards to another
		 * reference.
		 */
		object_reference(const object_id_type& _oid) :
				forwarded(true), ref_count(1), id(_oid)
		{
		}
		;

		object_reference(const object_reference& o)
		{
			forwarded = o.forwarded;
			ref_count = o.ref_count;

			if (forwarded)
				{
					id = o.id;
				}
			else
				{
					offset = o.offset;
				}
		}

	} object_reference_type;

	/** The type for the atom index. */
	typedef std::unordered_map<object_id_type, object_reference_type> index_map_type;

	/** The type of an atom. */
	typedef struct atom
	{
		// This is the column to object index. It maps a objectid to an
		// offset on the page.
		index_map_type index;

		// This is the actual data storage area.
		atom_data_type data;

		// The number of bytes written to this atom.
		atom_size_type size;

		atom() :
				size(0), data(
						atom_data_type::in | atom_data_type::out
								| atom_data_type::binary)
		{
		}
		;

	} atom_type;

	/** A handle to an atom. */
	typedef std::unique_ptr<atom_type> atom_handle_type;

	/** A list of atom handles. */
	typedef std::vector<atom_handle_type> atom_list_type;

	/** Give page cursor access to our internals. */
	friend class page_cursor;

private:
	//==----------------------------------------------------------==//
	//                        Data
	//==----------------------------------------------------------==//

	const atom_size_type k_max_atom_size = 1024 * 1024;

	/**
	 * This is the list of atoms.
	 */
	atom_list_type atoms;

	/**
	 * The maximum size an atom can attain.
	 */
	atom_size_type max_atom_size;

	/**
	 * The column definition.
	 */
	column_handle_type column;

	/**
	 * The next object id for this column.
	 */
	object_id_type next_oid;

	//==----------------------------------------------------------==//
	//                    Helper Functions
	//==----------------------------------------------------------==//

	/**
	 * Finds out where an object is located and returns that information.
	 *
	 * @param object_id: The object id to find.
	 *
	 * returns: A tuple of (result, atoms::iterator, index::iterator)
	 */
	std::tuple<bool, atom_list_type::iterator, index_map_type::iterator> find_object(
			object_id_type object_id)
	{
		for (auto atom = atoms.begin(); atom != atoms.end(); ++atom)
			{
				auto &index = (*atom)->index;
				auto pos = index.find(object_id);
				if (pos != index.end())
					{
						return std::make_tuple(true, atom, pos);
					}
			}

		return std::make_tuple(false, atoms.end(), atoms.back()->index.end());
	}

	/**
	 * Retrieves the last atom in the atoms list, inserting a new
	 * atom if the list is empty.
	 */
	atom_type* get_last_atom()
	{
		// If the atom is full, create a new one.
		if (atoms.size() == 0 || atoms.back()->size > max_atom_size)
			{
				atoms.push_back(atom_handle_type(new atom_type()));
			}

		return atoms.back().get();
	}

	/**
	 * Delegating constructor for building a page.
	 */
	page(atom_size_type _max_atom_size, cell::column* col) :
			max_atom_size(_max_atom_size), column(col), next_oid(1)
	{

	}

public:

	page() :
			page(k_max_atom_size, nullptr)
	{

	}

	page(cell::column *col) :
			page(k_max_atom_size, col)
	{

	}

	//==----------------------------------------------------------==//
	//                          API
	//==----------------------------------------------------------==//

	/**
	 * Set the maximum atom size.
	 *
	 * If the new size is zero, the current
	 * maximum size will not be set, and only
	 * the old size will be returned.
	 *
	 * @param new_size: The new size, in bytes, that
	 *                  an atom can store.
	 *
	 * @returns: The previous maximum atom size.
	 */
	atom_size_type set_max_atom_size(atom_size_type new_size)
	{
		auto old = max_atom_size;
		if (new_size > 0)
			{
				max_atom_size = new_size;
			}
		return old;
	}

	/**
	 * Counts how much data is stored in this page,
	 * and returns the value.
	 */
	std::uint64_t size()
	{
		std::uint64_t total_size = 0;

		for (int i = 0; i < atoms.size(); ++i)
			{
				total_size += atoms[i]->size;
			}

		return total_size;
	}

	/**
	 * Get the next object id for this column.
	 */
	object_id_type get_next_oid()
	{
		return next_oid++;
	}

	/**
	 * Gets the column definition for this page.
	 *
	 * You do not own this pointer. Do not store it,
	 * do not delete it.
	 */
	cell::column* get_column_definition()
	{
		return column.get();
	}

	/**
	 * Deletes the given object from this page.
	 *
	 * @param object_id: The object id to look for.
	 */
	void delete_object(object_id_type object_id)
	{
		bool result;
		atom_list_type::iterator atom;
		index_map_type::iterator pos;

		auto el = find_object(object_id);
		std::tie(result, atom, pos) = el;

		// If it was not found, return.
		if (!result)
			{
				return;
			}

		// Perform the deletion.
		pos->second.ref_count--;
		if (pos->second.ref_count == 0)
			{
				auto ap = (*atom).get();

				// Erase the object.
				ap->index.erase(pos);

				// Potentially erase the atom.
				// Note: this could use serious optimization.
				if (ap->size == 0)
					{
						atoms.erase(atom);
					}
			}
	}

	/**
	 * Deletes the given object from this page.
	 *
	 * @param objects: The set of objects to delete.
	 *
	 * @remarks: This function has not been optimized.
	 */
	void delete_objects(object_set_type objects)
	{
		for (auto object : objects)
			{
				delete_object(object);
			}
	}

	/**
	 * Acquires a reference count to this object. This is used
	 * by MVCC. A reference is removed by simply calling delete_object.
	 * The object will be released when its references are all done.
	 *
	 * @param object_id: The object id to acquire.
	 */
	bool acquire_object(object_id_type object_id)
	{
		bool result;
		atom_list_type::iterator atom;
		index_map_type::iterator pos;

		// If the atom is empty, we cannot read it.
		if (atoms.size() == 0)
			{
				return false;
			}

		// Get the object and unpack the results.
		auto el = find_object(object_id);
		std::tie(result, atom, pos) = el;

		// No object, bail.
		if (!result)
			{
				return false;
			}

		// Update the object's ref count.
		pos->second.ref_count++;

		return true;
	}

	/**
	 * Writes a new object into the page.
	 *
	 * @param object_id: The object to associate with the data.
	 * @param data: The data to write.
	 *
	 * @returns: The number of bytes written.
	 *
	 */
	template<typename T>
	size_type insert_object(object_id_type object_id, const T& data);

	/**
	 * Reads an existing object from the page.
	 *
	 * @param object_id: The object to associate with the data.
	 * @param data: The data to read.
	 *
	 * @returns: The number of bytes read.
	 *
	 * @note: This should only be used with basic data types.
	 */
	template<typename T>
	std::tuple<bool, size_type> fetch_object(object_id_type object_id, T& data);

	/**
	 * Provides access to the istream object on the page so that
	 * the object may be read directly.
	 *
	 * @param object_id: The object id to find.
	 *
	 * The istream will have its read head positioned correctly when
	 * returned.
	 */
	std::tuple<bool, std::istream*> get_stream(object_id_type object_id)
	{
		bool result;
		atom_list_type::iterator atom;
		index_map_type::iterator pos;

		// If the atom is empty, we cannot read it.
		if (atoms.size() == 0)
			{
				return std::make_tuple(false, nullptr);
			}

		// Get the object and unpack the results.
		auto el = find_object(object_id);
		std::tie(result, atom, pos) = el;

		if (!result)
			{
				return std::make_tuple(false, nullptr);
			}

		auto ap = (*atom).get();

		// Seek to the proper position
		ap->data.seekg(pos->second.offset);

		// Return success.
		return std::make_tuple(true, &(ap->data));
	}
};

/**
 * Tracks a page.
 */
typedef std::unique_ptr<page> page_handle_type;

//==----------------------------------------------------------==//
//                    Implementation
//==----------------------------------------------------------==//

/**
 * Writes a data element into a specific atom.
 *
 * Specialized for std::string.
 *
 * @param atom: The atom to write into.
 * @param data: The data to write.
 *
 */
template<typename T>
static void _insert_object(page::atom_type*atom, const T& data)
{
	// Write the data.
	atom->data.write(static_cast<const char*>(static_cast<const void*>(&data)),
			sizeof(data));

	atom->size += sizeof(data);
}

/**
 * Writes a data element into a specific atom.
 *
 * Specialized for std::string.
 *
 * @param atom: The atom to write into.
 * @param data: The data to write.
 *
 */
template<>
void _insert_object<>(page::atom_type*atom, const std::string& data)
{
	// Write the length of the string first.
	std::uint32_t size = data.size();
	_insert_object(atom, size);

	// Write the data.
	atom->data.write(data.c_str(), data.size());
	atom->size += size;
}

/**
 * Reads a data element from a specific atom.
 *
 * @param atom: The atom to read from.
 * @param data: The data to read.
 *
 */
template<typename T>
static page::size_type _fetch_object(page::atom_type*atom, T& data)
{
	// Read the data.
	atom->data.read(static_cast<char*>(static_cast<void*>(&data)), sizeof(data));

	return sizeof(data);
}

/**
 * Reads a data element from a specific atom.
 *
 * Specialized for std::string.
 *
 * @param atom: The atom to read from.
 * @param data: The data to read.
 *
 */
template<>
page::size_type _fetch_object<>(page::atom_type*atom, std::string& data)
{
	// Read the length of the string first.
	std::uint32_t size = data.size();
	_fetch_object(atom, size);

	char buf[size];

	// Read the data.
	atom->data.read(buf, size);

	// Copy it into the string.
	data.assign(buf, size);

	return size + sizeof(size);
}

/**
 * Writes a new object into the page.
 *
 * @param object_id: The object to associate with the data.
 * @param data: The data to write.
 *
 * @returns: The number of bytes written.
 *
 * @note: This should only be used with basic data types.
 * A complex data type (like string) may write correctly,
 * but its size will not be accurately recorded.
 */
template<typename T>
page::size_type page::insert_object(object_id_type object_id, const T& data)
{
	auto atom = get_last_atom();

	// Make sure we seek to the end of the stream.
	atom->data.seekp(0, std::ios::end);

	// Find out where that is.
	auto pos = atom->data.tellp();
	auto initial_size = atom->size;

	// Hand off the write.
	_insert_object(atom, data);

	// Update the index.
	atom->index.insert(
		{
		object_id, object_reference_type(pos)
		});

	return atom->size - initial_size;
}

/**
 * Reads an existing object from the page.
 *
 * @param object_id: The object to associate with the data.
 * @param data: The data to read.
 *
 * @returns: The number of bytes read.
 *
 * @note: This should only be used with basic data types.
 */
template<typename T>
std::tuple<bool, page::size_type> page::fetch_object(object_id_type object_id,
		T& data)
{
	bool result;
	atom_list_type::iterator atom;
	index_map_type::iterator pos;

	// If the atom is empty, we cannot read it.
	if (atoms.size() == 0)
		{
			return std::make_tuple(false, 0);
		}

	// Get the object and unpack the results.
	auto el = find_object(object_id);
	std::tie(result, atom, pos) = el;

	if (!result)
		{
			return std::make_tuple(false, 0);
		}

	auto ap = (*atom).get();

	// Seek to the proper position
	ap->data.seekg(pos->second.offset);

	// Return success.
	return std::make_tuple(true, _fetch_object(ap, data));
}

} // end namespace cell
} // end namespace lattice

#endif //__LATTICE_CELL_STORE_H__
