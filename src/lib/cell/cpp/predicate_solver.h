#ifndef __LATTICE_CELL_PREDICATE_H__
#define __LATTICE_CELL_PREDICATE_H__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
	namespace cell
   {

		class predicate
		{
		public:
			virtual int cmp(page_cursor& cursor)=0;
		};

   } // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PREDICATE_H__
