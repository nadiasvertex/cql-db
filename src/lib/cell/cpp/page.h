#ifndef __LATTICE_CELL_PAGE_H__
#define __LATTICE_CELL_PAGE_H__

#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace lattice
{
namespace cell
{

/**
 * A page contains data for a single column.
 */
class page {
public:
    typedef unsigned char byte_type;

    typedef unsigned long long row_id_type;

    typedef size_t size_type;

    typedef unsigned long atom_size_type;

    typedef std::set<row_id_type> row_set_type;

    typedef std::stringstream atom_data_type;

    typedef std::unordered_map<row_id_type, std::streampos> index_map_type;

    typedef struct atom {
        // This is the column to row index. It maps a rowid to an
        // offset on the page.
        index_map_type index;

        // This is the actual data storage area.
        atom_data_type data;

        // The number of bytes written to this atom.
        atom_size_type size;
    } atom_type;

    typedef std::unique_ptr<atom_type> atom_handle_type;

    typedef std::vector<atom_handle_type> atom_list_type;

private:
    /**
     * This is the list of atoms.
     */
    atom_list_type atoms;

    unsigned long max_atom_size;

public:

    page():max_atom_size(1024*1024) {

    }

    /**
     * Deletes the given row from this page.
     *
     * @param row_id: The row id to look for.
     */
    void delete_row(row_id_type row_id) {
        for(auto i=0; i<atoms.size(); ++i) {
            auto &index = atoms[i]->index;
            auto pos = index.find(row_id);
            if (pos!=index.end()) {
                index.erase(pos);
                break;
            }
        }
    }

    /**
     * Deletes the given row from this page.
     *
     * @param rows: The set of rows to delete.
     *
     * @remarks: This function has not been optimized.
     */
    void delete_rows(row_set_type rows) {
        for(auto row : rows) {
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
    size_type insert_row(row_id_type row_id, const T& data) {
        atom_type* atom { nullptr };

        // If the atom is full, create a new one.
        if (atoms.size()==0 || atoms.back()->size > max_atom_size) {
            atoms.push_back(atom_handle_type(new atom_type()));
            atoms.back()->size = 0;
        }

        atom = atoms.back().get();

        // Make sure we seek to the end of the stream.
        atom->data.seekp(0,std::ios::end);

        // Find out where that is.
        auto pos = atom->data.tellp();

        // Write the data.
        atom->data << data;
        atom->size += sizeof(data);

        // Update the index.
        atom->index[row_id] = pos;

        return sizeof(data);
    }







};

}
}

#endif //__LATTICE_CELL_STORE_H__
