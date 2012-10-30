#ifndef __LATTICE_CELL_PAGE_H__
#define __LATTICE_CELL_PAGE_H__

#include <iterator>
#include <pair>
#include <cell/cpp/page.h>

namespace lattice
{
namespace cell
{

  template<typename Container>
  class page_iterator : 
         public std::iterator<std::forward_iterator_tag,
	                      Container::value_type>
  {
  protected:
    Container& container;
  
  public:
    explicit page_iterator(Container& c)
    {
    }
  };	     


} // end namespace lattice
} // end namespace cell

#endif // __LATTICE_CELL_PAGE_H__
