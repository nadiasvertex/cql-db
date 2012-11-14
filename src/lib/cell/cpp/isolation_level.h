#ifndef __LATTICE_CELL_ISOLATION_LEVEL_H__
#define __LATTICE_CELL_ISOLATION_LEVEL_H__

namespace lattice {
namespace cell {

enum class isolation_level
{
   READ_UNCOMMITTED,
   READ_COMMITTED,
   REPEATABLE_READ
};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_ISOLATION_LEVEL_H__
