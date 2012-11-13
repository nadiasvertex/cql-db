#ifndef __LATTICE_CELL_ROW_ID_H__
#define __LATTICE_CELL_ROW_ID_H__

#include <cstdint>
#include <functional>

namespace lattice {
namespace cell {

class row_id
{
	std::uint64_t id;

   row_id(std::uint64_t _id):id(_id) {};

   friend class row_id_hash;

 public:

   row_id():id(0) {};

	static row_id from_uint64(std::uint64_t value)
	{
		return row_id(value);
	}

   row_id next()
	{
		return row_id(++id);
	}

	bool operator==(const row_id& o) const
	{
		return id == o.id;
	}

	bool operator<(const row_id& o) const
	{
		return id < o.id;
	}
};

class row_id_hash
{
public:
   std::size_t operator()(const row_id& rid) const
   {
      std::hash<std::uint64_t> h;
      return h(rid.id);
   }
};


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_ROW_ID_H__
