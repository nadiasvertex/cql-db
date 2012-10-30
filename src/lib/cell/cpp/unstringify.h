#ifndef __LATTICE_CELL_UNSTRINGIFY_H__
#define __LATTICE_CELL_UNSTRINGIFY_H__

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace lattice
{
namespace cell
{

class unstringify
{
public:
  static std::size_t to_binary(const std::string &input, std::int16_t *output)
  {
    const char *start = input.c_str();
    char *end;

    *output = static_cast<std::int16_t>(
        std::strtol(start, &end, 10)
        );

    return end-start;
  }

  static std::size_t to_binary(const std::string &input, std::int32_t *output)
  {
    const char *start = input.c_str();
    char *end;

    *output = static_cast<std::int32_t>(
        std::strtol(start, &end, 10)
        );

    return end-start;
  }

  static std::size_t to_binary(const std::string &input, std::int64_t *output)
  {
    const char *start = input.c_str();
    char *end;

    *output = static_cast<std::int64_t>(
        std::strtoll(start, &end, 10)
        );

    return end-start;
  }

  static std::size_t to_binary(const std::string &input, float *output)
  {
    const char *start = input.c_str();
    char *end;

    *output = std::strtof(start, &end);
    return end-start;
  }

  static std::size_t to_binary(const std::string &input, double *output)
  {
    const char *start = input.c_str();
    char *end;

    *output = std::strtod(start, &end);
    return end-start;
  }

  static std::size_t to_binary(const std::string &input, std::uint8_t *output)
  {
    std::uint32_t size = input.size();
    std::memcpy(output, &size, sizeof(size));
    output+=size;
    std::memcpy(output, input.c_str(), size);
    return sizeof(size)+size;
  }
};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_UNSTRINGIFY_H__
