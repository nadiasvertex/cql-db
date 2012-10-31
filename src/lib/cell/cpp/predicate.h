#ifndef __LATTICE_CELL_PREDICATE_H__
#define __LATTICE_CELL_PREDICATE_H__

#include <memory>

namespace lattice
{
	namespace cell
   {

	  class page_cursor;

		class predicate
		{
		public:
			virtual int cmp(page_cursor& cursor)=0;
			virtual bool contains(page_cursor& cursor)=0;
		};

		typedef std::shared_ptr<predicate> predicate_handle_type;

   } // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PREDICATE_H__
