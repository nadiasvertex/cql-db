#include <cell/cpp/scalar_predicate.h>

namespace lattice
{
namespace cell
{

template<>
void scalar_predicate::set_value<>(column::data_type t, const std::string& data)
{
  type = t;
  switch (type)
    {
    case column::data_type::varchar:
      value.s = new std::string(data);
      break;
    }
}


} // namespace cell
} // namespace lattice

