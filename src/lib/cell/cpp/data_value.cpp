#include <cell/cpp/scalar_predicate.h>

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

template<typename T>
void data_value::set_value(column::data_type t, const T& data)
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

} // namespace cell
} // namespace lattice

