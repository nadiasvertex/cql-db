#include <stdexcept>
#include <string>
#include <cell/cpp/data_value.h>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
namespace cell
{

/**
 * Provides a way to set a bit on
 * exit from a scope.
 */
class set_on_exit
{
	bool &v;
public:
	set_on_exit(bool&_v) :
			v(_v)
	{
	}

	~set_on_exit()
	{
		v = true;
	}
};

data_value::data_value(const data_value& o)
{
	type = o.type;
	has_value = o.has_value;
	switch (type)
		{
	case column::data_type::varchar:
		if (has_value)
			{
				value.s = new std::string(*o.value.s);
			}
		else
			{
				value.s = nullptr;
			}
		break;
	default:
		memcpy(&value, &(o.value), sizeof(value));
		}
}

data_value::~data_value()
{
	if (type == column::data_type::varchar && value.s != nullptr)
		{
			delete value.s;
		}
}

template<>
void data_value::set_value<>(column::data_type t, const std::string& data)
{
	set_on_exit f(has_value);

	type = t;
	switch (type)
		{
	case column::data_type::smallint:
		value.i16 = std::strtol(data.c_str(), NULL, 10);
		break;
	case column::data_type::integer:
		value.i32 = std::strtol(data.c_str(), NULL, 10);
		break;
	case column::data_type::bigint:
		value.i64 = std::strtoll(data.c_str(), NULL, 10);
		break;
	case column::data_type::real:
		value.f32 = std::strtod(data.c_str(), NULL);
		break;
	case column::data_type::double_precision:
		value.f64 = std::strtod(data.c_str(), NULL);
		break;
	case column::data_type::varchar:
		if (has_value)
			{
				delete value.s;
			}
		value.s = new std::string(data);
		break;
		}
}

bool data_value::operator==(const data_value& o) const
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

bool data_value::operator<(const data_value& o) const
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

int data_value::cmp(page_cursor& cursor) const
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

std::size_t data_value::write(std::uint8_t* buffer)
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

	value.assign(static_cast<char*>(static_cast<void*>(buffer + sizeof(size))),
			size);

	return size + sizeof(size);
}

std::size_t data_value::read(std::uint8_t* buffer)
{
	set_on_exit f(has_value);

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
		if (!has_value)
			{
				value.s = new std::string();
			}
		return _read(*value.s, buffer);
		}
}

template<typename T>
static std::size_t _write(const T& value, std::ostream& buffer)
{
	const char* ptr = static_cast<const char*>(static_cast<const void*>(&value));
	buffer.write(ptr, sizeof(value));

	return sizeof(value);
}

template<>
std::size_t _write<>(const std::string& value, std::ostream& buffer)
{
	std::uint32_t size = value.size();
	_write(size, buffer);

	buffer.write(value.c_str(), size);

	return size + sizeof(size);
}

std::size_t data_value::write(std::ostream& buffer)
{
	switch (type)
		{
	case column::data_type::smallint:
		return _write(value.i16, buffer);

	case column::data_type::integer:
		return _write(value.i32, buffer);

	case column::data_type::bigint:
		return _write(value.i64, buffer);

	case column::data_type::real:
		return _write(value.f32, buffer);

	case column::data_type::double_precision:
		return _write(value.f64, buffer);

	case column::data_type::varchar:
		return _write(*value.s, buffer);
		}
}

template<typename T>
static std::size_t _read(T& value, std::istream& buffer)
{
	char* ptr = static_cast<char*>(static_cast<void*>(&value));
	buffer.read(ptr, sizeof(value));

	return sizeof(value);
}

template<>
std::size_t _read<>(std::string& value, std::istream& buffer)
{
	std::uint32_t size = value.size();
	_read(size, buffer);

	char _buffer[size];
	buffer.read(_buffer, size);
	value.assign(_buffer, size);

	return size + sizeof(size);
}

std::size_t data_value::read(std::istream& buffer)
{
	switch (type)
		{
	case column::data_type::smallint:
		return _read(value.i16, buffer);

	case column::data_type::integer:
		return _read(value.i32, buffer);

	case column::data_type::bigint:
		return _read(value.i64, buffer);

	case column::data_type::real:
		return _read(value.f32, buffer);

	case column::data_type::double_precision:
		return _read(value.f64, buffer);

	case column::data_type::varchar:
		return _read(*value.s, buffer);
		}
}

data_value data_value::as_smallint() const
{
	data_value out(column::data_type::smallint);

	switch (type)
		{
	case column::data_type::smallint:
		out.value.i16 = value.i16;
		break;

	case column::data_type::integer:
		out.value.i16 = value.i32;
		break;

	case column::data_type::bigint:
		out.value.i16 = value.i64;
		break;

	case column::data_type::real:
		out.value.i16 = std::int16_t(value.f32);
		break;

	case column::data_type::double_precision:
		out.value.i16 = std::int16_t(value.f64);
		break;

	case column::data_type::varchar:
		out.set_value(column::data_type::smallint, *value.s);
		break;
		}

	return out;
}

data_value data_value::as_integer() const
{
	data_value out(column::data_type::integer);

	switch (type)
		{
	case column::data_type::smallint:
		out.value.i32 = value.i16;
		break;

	case column::data_type::integer:
		out.value.i32 = value.i32;
		break;

	case column::data_type::bigint:
		out.value.i32 = value.i64;
		break;

	case column::data_type::real:
		out.value.i32 = std::int32_t(value.f32);
		break;

	case column::data_type::double_precision:
		out.value.i32 = std::int32_t(value.f64);
		break;

	case column::data_type::varchar:
		out.set_value(column::data_type::integer, *value.s);
		break;
		}

	return out;
}

data_value data_value::as_bigint() const
{
	data_value out(column::data_type::bigint);

	switch (type)
		{
	case column::data_type::smallint:
		out.value.i64 = value.i16;
		break;

	case column::data_type::integer:
		out.value.i64 = value.i32;
		break;

	case column::data_type::bigint:
		out.value.i64 = value.i64;
		break;

	case column::data_type::real:
		out.value.i64 = std::int64_t(value.f32);
		break;

	case column::data_type::double_precision:
		out.value.i64 = std::int64_t(value.f64);
		break;

	case column::data_type::varchar:
		out.set_value(column::data_type::bigint, *value.s);
		break;
		}

	return out;
}

data_value data_value::as_real() const
{
	data_value out(column::data_type::real);

	switch (type)
		{
	case column::data_type::smallint:
		out.value.f32 = float(value.i16);
		break;

	case column::data_type::integer:
		out.value.f32 = float(value.i32);
		break;

	case column::data_type::bigint:
		out.value.f32 = float(value.i64);
		break;

	case column::data_type::real:
		out.value.f32 = float(value.f32);
		break;

	case column::data_type::double_precision:
		out.value.f32 = float(value.f64);
		break;

	case column::data_type::varchar:
		out.set_value(column::data_type::real, *value.s);
		break;
		}

	return out;
}

data_value data_value::as_double_precision() const
{
	data_value out(column::data_type::double_precision);

	switch (type)
		{
	case column::data_type::smallint:
		out.value.f64 = double(value.i16);
		break;

	case column::data_type::integer:
		out.value.f64 = double(value.i32);
		break;

	case column::data_type::bigint:
		out.value.f64 = double(value.i64);
		break;

	case column::data_type::real:
		out.value.f64 = double(value.f32);
		break;

	case column::data_type::double_precision:
		out.value.f64 = double(value.f64);
		break;

	case column::data_type::varchar:
		out.set_value(column::data_type::double_precision, *value.s);
		break;
		}

	return out;
}

data_value data_value::as_string() const
{
	data_value out(column::data_type::varchar);
	out.has_value = true;

	switch (type)
		{
	case column::data_type::smallint:
		out.value.s = new std::string(std::to_string(value.i16));
		break;

	case column::data_type::integer:
		out.value.s = new std::string(std::to_string(value.i32));
		break;

	case column::data_type::bigint:
		out.value.s = new std::string(std::to_string(value.i64));
		break;

	case column::data_type::real:
		out.value.s = new std::string(std::to_string(value.f32));
		break;

	case column::data_type::double_precision:
		out.value.s = new std::string(std::to_string(value.f64));
		break;

	case column::data_type::varchar:
		out.value.s = new std::string(*(value.s));
		break;

	default:
		out.value.s = nullptr;
		out.has_value = false;
		}

	return out;
}

expected<data_value> operator+(const data_value& l, const data_value& r)
{
	data_value out;

	switch (l.type)
		{
	case column::data_type::smallint:
		out.set_value(l.type, l.value.i16 + r.as_smallint().value.i16);
		break;
	case column::data_type::integer:
		out.set_value(l.type, l.value.i32 + r.as_integer().value.i32);
		break;
	case column::data_type::bigint:
		out.set_value(l.type, l.value.i64 + r.as_bigint().value.i64);
		break;
	case column::data_type::real:
		out.set_value(l.type, l.value.f32 + r.as_real().value.f32);
		break;
	case column::data_type::double_precision:
		out.set_value(l.type, l.value.f64 + r.as_double_precision().value.f64);
		break;
	case column::data_type::varchar:
		{
			auto* ls = l.value.s;
			auto* rs = r.as_string().value.s;
			out.set_value(l.type, (*ls) + (*rs));
		}
		break;
	default:
		return expected<data_value>::from_exception(
				std::runtime_error("operator '+' called for unknown type."));
		}

	return expected<data_value>(out);
}

expected<data_value> operator-(const data_value& l, const data_value& r)
{
	data_value out;

	switch (l.type)
		{
	case column::data_type::smallint:
		out.set_value(l.type, l.value.i16 - r.as_smallint().value.i16);
		break;
	case column::data_type::integer:
		out.set_value(l.type, l.value.i32 - r.as_integer().value.i32);
		break;
	case column::data_type::bigint:
		out.set_value(l.type, l.value.i64 - r.as_bigint().value.i64);
		break;
	case column::data_type::real:
		out.set_value(l.type, l.value.f32 - r.as_real().value.f32);
		break;
	case column::data_type::double_precision:
		out.set_value(l.type, l.value.f64 - r.as_double_precision().value.f64);
		break;
	case column::data_type::varchar:
		return expected<data_value>::from_exception(
				std::runtime_error(
						"operator '-' is not valid for string literals."));
		break;
	default:
		return expected<data_value>::from_exception(
				std::runtime_error("operator '-' called for unknown type."));
		}

	return expected<data_value>(out);
}

expected<data_value> operator*(const data_value& l, const data_value& r)
{
	data_value out;

	switch (l.type)
		{
	case column::data_type::smallint:
		out.set_value(l.type, l.value.i16 * r.as_smallint().value.i16);
		break;
	case column::data_type::integer:
		out.set_value(l.type, l.value.i32 * r.as_integer().value.i32);
		break;
	case column::data_type::bigint:
		out.set_value(l.type, l.value.i64 * r.as_bigint().value.i64);
		break;
	case column::data_type::real:
		out.set_value(l.type, l.value.f32 * r.as_real().value.f32);
		break;
	case column::data_type::double_precision:
		out.set_value(l.type, l.value.f64 * r.as_double_precision().value.f64);
		break;
	case column::data_type::varchar:
		return expected<data_value>::from_exception(
				std::runtime_error(
						"operator '*' is not valid for string literals."));
		break;
	default:
		return expected<data_value>::from_exception(
				std::runtime_error("operator '*' called for unknown type."));
		}

	return expected<data_value>(out);
}

expected<data_value> operator/(const data_value& l, const data_value& r)
{
	data_value out;

	switch (l.type)
		{
	case column::data_type::smallint:
		out.set_value(l.type, l.value.i16 / r.as_smallint().value.i16);
		break;
	case column::data_type::integer:
		out.set_value(l.type, l.value.i32 / r.as_integer().value.i32);
		break;
	case column::data_type::bigint:
		out.set_value(l.type, l.value.i64 / r.as_bigint().value.i64);
		break;
	case column::data_type::real:
		out.set_value(l.type, l.value.f32 / r.as_real().value.f32);
		break;
	case column::data_type::double_precision:
		out.set_value(l.type, l.value.f64 / r.as_double_precision().value.f64);
		break;
	case column::data_type::varchar:
		return expected<data_value>::from_exception(
				std::runtime_error(
						"operator '/' is not valid for string literals."));
		break;
	default:
		return expected<data_value>::from_exception(
				std::runtime_error("operator '/' called for unknown type."));
		}

	return expected<data_value>(out);
}

} // namespace cell
} // namespace lattice

