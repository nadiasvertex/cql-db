#ifndef __LATTICE_CELL_SCALAR_DATA_VALUE_H__
#define __LATTICE_CELL_SCALAR_DATA_VALUE_H__

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <cell/cpp/predicate_solver.h>

namespace lattice
{
namespace cell
{

class page_cursor;

class data_value
{
  /**
   * The type of data to request.
   */
  column::data_type type;

  bool has_value;

  union
  {
    std::int16_t i16;
    std::int32_t i32;
    std::int64_t i64;

    float f32;
    double f64;

    std::string *s;

  } value;

public:
  data_value() :
      has_value(false)
  {
  }

  data_value(column::data_type t) :
      type(t), has_value(false)
  {
  }

  /**
   * Cleans up resources, like string pointers.
   */
  ~data_value();

  /**
   * Set the value of the data contained.
   *
   * @param t: The type of data being set.
   * @param data: The data to set. This will be converted if it
   *              is reasonably close to C++ conversion rules for
   *              data types.
   */
  template<typename T>
  void set_value(column::data_type t, const T& data)
  {
    type = t;
    switch (type)
      {
      case column::data_type::smallint:
        value.i16 = data;
        break;

      case column::data_type::integer:
        value.i32 = data;
        break;

      case column::data_type::bigint:
        value.i32 = data;
        break;

      case column::data_type::real:
        value.f32 = data;
        break;

      case column::data_type::double_precision:
        value.f64 = data;
        break;
      }

    has_value = true;
  }

  /**
   * Write this data value into a buffer.
   *
   * @param buffer: The buffer to write into.
   */
  std::size_t write(std::uint8_t* buffer);

  /**
   * Read this data value from a buffer.
   *
   * @param buffer: The buffer to read from.
   */
  std::size_t read(std::uint8_t* buffer);

  /**
   * Write this data value into a buffer.
   *
   * @param buffer: The buffer to write into.
   */
  std::size_t write(std::ostream& buffer);

  /**
   * Read this data value from a buffer.
   *
   * @param buffer: The buffer to read from.
   */
  std::size_t read(std::istream& buffer);

  /**
   * Compares the data value with the current value
   * pointed to by the cursor.
   *
   * @param cursor: The page cursor to compare with.
   *
   */
  int cmp(page_cursor& cursor) const;

  /**
   * Establishes data equivalency for data values.
   */
  bool operator==(const data_value& o) const;

  /**
   *  Establishes less than ordering for data values.
   */
  bool operator<(const data_value& o) const;
};

template<>
  void data_value::set_value<>(column::data_type t, const std::string& data);


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_DATA_VALUE_H__
