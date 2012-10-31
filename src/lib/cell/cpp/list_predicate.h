#ifndef __LATTICE_CELL_LIST_PREDICATE_H__
#define __LATTICE_CELL_LIST_PREDICATE_H__

#include <set>

#include <cell/cpp/predicate_solver.h>
#include <cell/cpp/data_value.h>

namespace lattice
{
  namespace cell
  {

	 class list_predicate: public predicate
	 {
	 public:
		typedef std::set<data_value> scalar_list_type;
	 private:
		scalar_list_type pred_list;
	 public:
		template<typename T>
		  void add_value(column::data_type t, const T& value)
		  {
			 data_value pred;
			 pred.set_value(t, value);
			 auto p =  pred_list.insert(pred);
			 
		  }

		virtual int cmp(page_cursor& cursor);

		virtual bool contains(page_cursor& cursor);
	 };

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_LIST_PREDICATE_H__
