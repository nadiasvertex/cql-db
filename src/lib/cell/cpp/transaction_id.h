#ifndef __LATTICE_CELL_TRANSACTION_ID_H__
#define __LATTICE_CELL_TRANSACTION_ID_H__

#include <cstdint>

namespace lattice {
namespace cell {

class transaction_id
{
	std::uint64_t id;

   transaction_id(std::uint64_t _id):id(_id) {};

   friend class transaction_id_hash;
 public:

   transaction_id():id(0) {};

	static transaction_id from_uint64(std::uint64_t value)
	{
		return transaction_id(value);
	}

   transaction_id next()
	{
		return transaction_id(++id);
	}

   void reset()
   {
      id = 0;
   }

   bool empty() const
   {
      return id == 0;
   }

	bool operator==(const transaction_id& o) const
	{
		return id == o.id;
	}

	bool operator<(const transaction_id& o) const
	{
		return id < o.id;
	}
};

class transaction_id_hash
{
public:
   std::size_t operator()(const transaction_id& tid) const
   {
      std::hash<std::uint64_t> h;
      return h(tid.id);
   }
};



} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_TRANSACTION_ID_H__
