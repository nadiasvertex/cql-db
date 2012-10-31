#include <cell/cpp/data_value.h>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
namespace cell
{

data_value::~data_value()
{
  if (type == column::data_type::varchar)
    {
      delete value.s;
    }
}

template<>
void data_value::set_value<>(column::data_type t, const std::string& data)
{
  type = t;
  switch (type)
    {
    case column::data_type::varchar:
      if (has_value)
        {
          delete value.s;
        }
      value.s = new std::string(data);
      break;
    }

  has_value = true;
}

bool data_value::operator==(data_value o)
{
  switch (type)
    {
    case column::data_type::smallint:
      return value.i16 == o.value.i16;

    case column::data_type::integer:
      return value.i32 == o.value.i32;

    case column::data_type::bigint:
      return value.i64 == o.value.i64;

    case column::data_type::real:
      return value.f32 == o.value.f32;

    case column::data_type::double_precision:
      return value.f64 == o.value.f64;

    case column::data_type::varchar:
      return *value.s == *o.value.s;
    }

  return false;
}

bool data_value::operator<(data_value o)
{
  switch (type)
    {
    case column::data_type::smallint:
      return value.i16 < o.value.i16;

    case column::data_type::integer:
      return value.i32 < o.value.i32;

    case column::data_type::bigint:
      return value.i64 < o.value.i64;

    case column::data_type::real:
      return value.f32 < o.value.f32;

    case column::data_type::double_precision:
      return value.f64 < o.value.f64;

    case column::data_type::varchar:
      return *value.s < *o.value.s;
    }

  return false;
}

int data_value::cmp(page_cursor& cursor)
{
  switch (type)
    {
    case column::data_type::smallint:
      return cursor.cmp(value.i16);
      break;

    case column::data_type::integer:
      return cursor.cmp(value.i32);
      break;

    case column::data_type::bigint:
      return cursor.cmp(value.i32);
      break;

    case column::data_type::real:
      return cursor.cmp(value.f32);
      break;

    case column::data_type::double_precision:
      return cursor.cmp(value.f64);
      break;

    case column::data_type::varchar:
      return cursor.cmp(*value.s);
      break;
    }
}

template<typename T>
static std::size_t _write(const T& value, std::uint8_t* buffer)
{
  T* ptr = static_cast<T*>(static_cast<void*>(buffer));
  *ptr = value;

  return sizeof(value);
}

template<>
std::size_t _write<>(const std::string& value, std::uint8_t* buffer)
{
  std::uint32_t size = value.size();
  _write(size, buffer);

  std::memcpy(buffer + sizeof(size), value.c_str(), size);

  return size + sizeof(size);
}

std::size_t 
data_value::write(std::uint8_t* buffer)
{
  switch (type)
    {
    case column::data_type::smallint:
      return _write(value.i16, buffer);

    case column::data_type::integer:
      return _write(value.i32, buffer);

    case column::data_type::bigint:
      return _write(value.i32, buffer);

    case column::data_type::real:
      return _write(value.f32, buffer);

    case column::data_type::double_precision:
      return _write(value.f64, buffer);

    case column::data_type::varchar:
      return _write(*value.s, buffer);
    }
}

template<typename T>
static std::size_t _read(T& value, std::uint8_t* buffer)
{
  T* ptr = static_cast<T*>(static_cast<void*>(buffer));
  value = *ptr;

  return sizeof(value);
}

template<>
std::size_t _read<>(std::string& value, std::uint8_t* buffer)
{
  std::uint32_t size = 0;
  _read(size, buffer);

  value.assign(static_cast<char*>(
			static_cast<void*>(buffer + sizeof(size))
											 ), size);

  return size + sizeof(size);
}


std::size_t 
data_value::read(std::uint8_t* buffer)
{
    switch (type)
    {
    case column::data_type::smallint:
      return _read(value.i16, buffer);

    case column::data_type::integer:
      return _read(value.i32, buffer);

    case column::data_type::bigint:
      return _read(value.i32, buffer);

    case column::data_type::real:
      return _read(value.f32, buffer);

    case column::data_type::double_precision:
      return _read(value.f64, buffer);

    case column::data_type::varchar:
      return _read(*value.s, buffer);
    }
}

std::size_t write(std::ostream& buffer)
{

}

std::size_t read(std::istream& buffer)
{

}

} // namespace cell
} // namespace lattice

