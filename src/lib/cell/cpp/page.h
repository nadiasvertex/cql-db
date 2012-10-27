#ifndef __LATTICE_CELL_PAGE_H__
#define __LATTICE_CELL_PAGE_H__

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace lattice
{
namespace cell
{

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

  /** The type storing how large an atom can be. */
  typedef unsigned long atom_size_type;

  /** The type for object sets. */
  typedef std::set<object_id_type> object_set_type;

  /** The type for atom data bulk storage. */
  typedef std::stringstream atom_data_type;

  /** The type for the atom index. */
  typedef std::unordered_map<object_id_type, std::streampos> index_map_type;

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
            atom_data_type::in | atom_data_type::out | atom_data_type::binary)
    {
    }
    ;
  } atom_type;

  /** A handle to an atom. */
  typedef std::unique_ptr<atom_type> atom_handle_type;

  /** A list of atom handles. */
  typedef std::vector<atom_handle_type> atom_list_type;

private:
  //==----------------------------------------------------------==//
  //                        Data
  //==----------------------------------------------------------==//

  /**
   * This is the list of atoms.
   */
  atom_list_type atoms;

  /**
   * The maximum size an atom can attain.
   */
  atom_size_type max_atom_size;

  //==----------------------------------------------------------==//
  //                    Helper Functions
  //==----------------------------------------------------------==//

  std::tuple<bool, atom_type*, index_map_type::iterator> find_object(
      object_id_type object_id)
  {
    for (auto i = 0; i < atoms.size(); ++i)
      {
        auto &index = atoms[i]->index;
        auto pos = index.find(object_id);
        if (pos != index.end())
          {
            return std::make_tuple(true, atoms[i].get(), pos);
          }
      }

    return std::make_tuple(false, nullptr, atoms.back()->index.end());
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

public:

  page() :
      max_atom_size(1024 * 1024)
  {

  }

  //==----------------------------------------------------------==//
  //                          API
  //==----------------------------------------------------------==//

  /** Set the maximum atom size.
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
   * Deletes the given object from this page.
   *
   * @param object_id: The object id to look for.
   */
  void delete_object(object_id_type object_id)
  {
    bool result;
    atom_type* atom;
    index_map_type::iterator pos;

    auto el = find_object(object_id);
    std::tie(result, atom, pos) = el;

    // If it was not found, return.
    if (!result)
      {
        return;
      }

    // Perform the deletion.
    atom->index.erase(pos);

    // Potentially erase the atom.
    if (atom->size == 0)
      {
        for (auto p = atoms.begin(); p != atoms.end(); ++p)
          {
            if (p->get() == atom)
              {
                atoms.erase(p);
                break;
              }
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
};

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
void
_insert_object(page::atom_type*atom, const T& data)
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
void
_insert_object<>(page::atom_type*atom, const std::string& data)
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
page::size_type
_fetch_object(page::atom_type*atom, T& data)
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
page::size_type
_fetch_object<>(page::atom_type*atom, std::string& data)
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
page::size_type
page::insert_object(object_id_type object_id, const T& data)
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
  atom->index[object_id] = pos;

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
std::tuple<bool, page::size_type>
page::fetch_object(object_id_type object_id, T& data)
{
  bool result;
  atom_type* atom;
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

  // Seek to the proper position
  atom->data.seekg(pos->second);

  // Return success.
  return std::make_tuple(true, _fetch_object(atom, data));
}

}
}

#endif //__LATTICE_CELL_STORE_H__
