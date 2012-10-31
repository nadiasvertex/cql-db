#ifndef __LATTICE_CELL_SCALAR_PREDICATE_H__
#define __LATTICE_CELL_SCALAR_PREDICATE_H__

#include <cstdint>
#include <memory>
#include <string>

#include <cell/cpp/data_value.h>
#include <cell/cpp/predicate_solver.h>

namespace lattice
{
namespace cell
{

class scalar_predicate: public predicate
{
  data_value value;

public:
  scalar_predicate()
  {
  }

  template <typename T>
  void set_value(column::data_type t, const T& v) {
    value.set_value(t, v);
  }

  virtual int cmp(page_cursor& cursor)
  {
    return value.cmp(cursor);
  }

  virtual bool contains(page_cursor& cursor)
  {
    return false;
  }
};


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_SCALAR_PREDICATE_H__
