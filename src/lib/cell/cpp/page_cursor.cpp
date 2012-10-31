
#include <cell/cpp/data_value.h>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
namespace cell
{

data_value 
page_cursor::get_value() 
  {
	 auto type = p.get_column_definition()->type;
	 data_value v(type);

	 auto atom = p.atoms[atom_index].get();

    // Seek to the proper position
    atom->data.seekg(entry->second.offset);

	 // Read out the data value
	 v.read(atom->data);

	 return v;
  }



} //end namespace cell
} //end namespace lattice


