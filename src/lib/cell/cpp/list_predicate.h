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

class list_predicate : public predicate
  {
  public:
    typedef std::vector<scalar_predicate> scalar_list_type;
  private:
    scalar_list_type pred_list;
  public:
    template<typename T>
      void add_value(const T& value)
      {
        pred_list.emplace_back();
        pred_list.back().set_value(value);

      }
  };


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_LIST_PREDICATE_H__
