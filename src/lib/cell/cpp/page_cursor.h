#ifndef __LATTICE_CELL_PAGE_CURSOR_H__
#define __LATTICE_CELL_PAGE_CURSOR_H__

#include <cell/cpp/page.h>

namespace lattice
{
namespace cell
{

class page_cursor
{
  /**
   * The page we are attached to.
   */
  page& p;

  /**
   * Which atom we are iterating.
   */
  unsigned int atom_index;

  /**
   * The atom index iterator.
   */
  page::index_map_type::iterator entry;

  /**
   * When this is set to true, we have reached the end of the page.
   */
  bool at_end;

  public:

  page_cursor(page& _page) :
      p(_page),
      atom_index(0),
      at_end(false)
  {
    if (p.atoms.size() > 0)
      {
        entry = p.atoms[0]->index.begin();
      }
  }

  /**
   * Indicates when we are at the end of the page.
   */
  bool end_of_page()
  {
    return at_end;
  }

  /**
   * Advance the cursor one column.
   */
  page_cursor& advance()
  {
    if (!at_end)
      {
        ++entry;
        if (entry == p.atoms[atom_index]->index.end())
          {
            ++atom_index;
            if (atom_index < p.atoms.size())
              {
                entry = p.atoms[atom_index]->index.begin();
              }
            else
              {
                at_end=true;
              }
          }
        else
          {
            at_end=true;
          }
      }
    return *this;
  }

  /**
   * Get the oid of the object that the cursor
   * is currently pointing to.
   *
   * If the cursor is at the end of the page, the
   * result of this function is undefined.
   */
  page::object_id_type oid() {
    return entry->first;
  }

  /**
   * Get the value of the data at the cursor position.
   *
   * If the cursor is at the end of the page, the
   * result of this function is undefined.
   *
   * @param data: The data will be written into this object.
   *
   * @returns: The number of bytes written to 'data'.
   */
  template <typename T>
  std::size_t value(T& data) {
    auto atom = p.atoms[atom_index].get();

    // Seek to the proper position
    atom->data.seekg(entry->second.offset);

    return _fetch_object(atom, data);
  }

};

} // end namespace cell
} // end namespace lattice

#endif //__LATTICE_CELL_PAGE_CURSOR_H__
