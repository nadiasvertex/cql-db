#include <cell/cpp/list_predicate.h>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
namespace cell
{

int
list_predicate::cmp(page_cursor& cursor)
{
  return 0;
}

bool
list_predicate::contains(page_cursor& cursor)
{
  auto value = cursor.get_value();
  return pred_list.find(value) != pred_list.end();
}

} // namespace cell
} // namespace lattice
