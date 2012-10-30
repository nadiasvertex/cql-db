#ifndef __LATTICE_CELL_COMPARE_H__
#define __LATTICE_CELL_COMPARE_H__

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <istream>

namespace lattice
{
namespace cell
{

class compare
{
public:
  template<typename T>
  static int cmp(const T& value, const T* buffer)
  {
    return value == (*buffer) ? 0 :
           value > (*buffer) ? 1 : -1;
  }

  template<typename T>
  static int cmp(const T& value, std::istream& buffer)
  {
    T value2;
    buffer.read(static_cast<void*>(&value2), sizeof(value2));

    return value == value2 ? 0 :
           value > value2 ? 1 : -1;
  }

  template<typename T>
  static int cmp(const T* value, const T* buffer)
  {
    return (*value) == (*buffer) ? 0 :
           (*value) > (*buffer) ? 1 : -1;
  }

  template<>
  static int cmp<>(const std::string &value, std::uint8_t *buffer)
  {
    std::uint32_t size1 = input.size();
    std::uint32_t size2 = *static_cast<std::uint32_t>(
        static_cast<void*>(buffer)
        );

    // Step forward to the strings.
    buffer += sizeof(size1);

    // Find the minimum size.
    auto size = std::min(size1, size2);

    // Compare
    auto r = memcmp(buffer, value.c_str(), size);

    // If the strings compared equivalent, and they
    // are not the same size, make sure that we figure
    // out the proper ordering.
    if (r == 0 && size1 != size2)
      {
        return size1 > size2 ? 1 : -1;
      }

    return r;
  }

  template<>
  static int cmp<>(const std::string& value, std::istream& buffer)
  {
    std::uint32_t size1 = value.size();
    std::uint32_t size2 = 0;

    // Get the size
    buffer.read(static_cast<void*>(&size2), sizeof(size2));

    // Find the minimum size
    auto size = std::min(size1, size2);

    // Stream the comparison from the buffer
    for (auto i = 0; i < size; ++size)
      {
        auto c1 = value[i];
        auto c2 = buffer.get();

        if (c1 == c2)
          {
            continue;
          }

        if (c1 > c2)
          {
            return 1;
          }

        return -1;
      }

    // If the strings compared equivalent, and they
    // are not the same size, make sure that we figure
    // out the proper ordering.
    return (size1 == size2) ? 0 :
           (size1 >  size2) ? 1 : -1;
  }

}
;

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_COMPARE_H__
