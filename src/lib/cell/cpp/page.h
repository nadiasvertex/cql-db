#ifndef __LATTICE_CELL_PAGE_H__
#define __LATTICE_CELL_PAGE_H__

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
    typedef unsigned char byte_type;

    typedef unsigned long long row_id_type;

    typedef size_t size_type;

    typedef unsigned long atom_size_type;

    typedef std::set<row_id_type> row_set_type;

    typedef std::stringstream atom_data_type;

    typedef std::unordered_map<row_id_type, std::streampos> index_map_type;

    typedef struct atom
    {
        // This is the column to row index. It maps a rowid to an
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

    typedef std::unique_ptr<atom_type> atom_handle_type;

    typedef std::vector<atom_handle_type> atom_list_type;

private:
    /**
     * This is the list of atoms.
     */
    atom_list_type atoms;

    unsigned long max_atom_size;

    std::tuple<bool, atom_type*, index_map_type::iterator> find_row(
            row_id_type row_id)
    {
        for (auto i = 0; i < atoms.size(); ++i)
            {
                auto &index = atoms[i]->index;
                auto pos = index.find(row_id);
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
    void _insert_row(atom_type*atom, const T& data)
    {
        // Write the data.
        atom->data.write(
               static_cast<const char*>(
                  static_cast<const void*>(&data)
                ), sizeof(data)
        );

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
    void _insert_row(atom_type*atom, const std::string& data)
    {
        // Write the data.
        atom->data.write(data.c_str(), data.size());
        atom->size += data.size();
    }

public:

    page() :
            max_atom_size(1024 * 1024)
    {

    }

    /**
     * Deletes the given row from this page.
     *
     * @param row_id: The row id to look for.
     */
    void delete_row(row_id_type row_id)
    {
        bool result;
        atom_type* atom;
        index_map_type::iterator pos;

        auto el = find_row(row_id);
        std::tie(result, atom, pos) = el;

        // If it was not found, return.
        if (!result)
            {
                return;
            }

        // Perform the deletion.
        atom->index.erase(pos);
    }

    /**
     * Deletes the given row from this page.
     *
     * @param rows: The set of rows to delete.
     *
     * @remarks: This function has not been optimized.
     */
    void delete_rows(row_set_type rows)
    {
        for (auto row : rows)
            {
                delete_row(row);
            }
    }

    /**
     * Writes a new row into the page.
     *
     * @param row_id: The row to associate with the data.
     * @param data: The data to write.
     *
     * @returns: The number of bytes written.
     *
     * @note: This should only be used with basic data types.
     * A complex data type (like string) may write correctly,
     * but its size will not be accurately recorded.
     */
    template<typename T>
    size_type insert_row(row_id_type row_id, const T& data)
    {
        auto atom = get_last_atom();

        // Make sure we seek to the end of the stream.
        atom->data.seekp(0, std::ios::end);

        // Find out where that is.
        auto pos = atom->data.tellp();

        // Hand off the write.
        _insert_row(atom, data);

        // Update the index.
        atom->index[row_id] = pos;

        return sizeof(data);
    }

    /**
     * Reads an existing row from the page.
     *
     * @param row_id: The row to associate with the data.
     * @param data: The data to read.
     *
     * @returns: The number of bytes read.
     *
     * @note: This should only be used with basic data types.
     */
    template<typename T>
    std::tuple<bool, size_type> fetch_row(row_id_type row_id, T& data)
    {

        // If the atom is empty, we cannot read it.
        if (atoms.size() == 0)
            {
                return std::make_tuple(false, 0);
            }

        bool result;
        atom_type* atom;
        index_map_type::iterator pos;

        auto el = find_row(row_id);
        std::tie(result, atom, pos) = el;

        if (!result)
            {
                return std::make_tuple(false, 0);
            }

        // Seek to the proper position
        atom->data.seekg(pos->second);

        // Read the data.
        atom->data.read(static_cast<char*>(static_cast<void*>(&data)),
                sizeof(data));

        // Return success.
        return std::make_tuple(true, sizeof(data));
    }

};

}
}

#endif //__LATTICE_CELL_STORE_H__
