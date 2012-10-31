#ifndef __LATTICE_CELL_LIST_PREDICATE_H__
#define __LATTICE_CELL_LIST_PREDICATE_H__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <cell/cpp/scalar_predicate.h>

namespace lattice
{
namespace cell
{

class list_predicate: public predicate
{
public:
  typedef std::vector<scalar_predicate> scalar_list_type;
  private:
  scalar_list_type pred_list;
  public:
  template<typename T>
  void add_value(column::data_type t, const T& value)
  {
    pred_list.emplace_back();
    pred_list.back().set_value(t, value);
  }

  virtual int cmp(page_cursor& cursor)
  {
    return 0;
  }

  virtual bool contains(page_cursor& cursor)
  {
    for(auto p : pred_list)
      {
        if (p.cmp(cursor) == 0) {
            return true;
        }
      }

    return false;
  }
};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_LIST_PREDICATE_H__
