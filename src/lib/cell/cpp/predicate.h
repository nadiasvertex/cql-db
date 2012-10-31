#ifndef __LATTICE_CELL_PREDICATE_H__
#define __LATTICE_CELL_PREDICATE_H__

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

   } // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_PREDICATE_H__
