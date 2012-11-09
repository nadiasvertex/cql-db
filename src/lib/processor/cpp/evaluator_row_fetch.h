/**
 * The file contains row-fetch forwarding for the JIT code. The JIT always
 * knows what kind of data lives in the columns that it's fetching because
 * the schema has not changed. However, it doesn't have access to the C++
 * vtable dispatch mechanism, so these functions perform the lookup and
 * dispatch services.
 *
 * This file is built to be included directly in evaluator.cpp.
 */

static std::int16_t fetch_int16_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_int16_value();
}

static std::int32_t fetch_int32_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_int32_value();
}

static std::int64_t fetch_int64_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_int64_value();
}

static float fetch_float_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_float_value();
}

static double fetch_double_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_double_value();
}

static std::string* fetch_string_value(row_buffer* rb, int column_index)
{
	return rb->get_current_row()[column_index].raw_string_value();
}

static std::string*
convert_value_to_string(cell::column::data_type type, void* value)
{
	cell::data_value dv;

	switch (type)
		{
		case cell::column::data_type::smallint:
			{
				auto* i16 = static_cast<std::int16_t*>(value);
				dv.set_value(type, *i16);
				return new std::string(dv.to_string());
			}

		case cell::column::data_type::integer:
			{
				auto* i32 = static_cast<std::int32_t*>(value);
				dv.set_value(type, *i32);
				return new std::string(dv.to_string());
			}

		case cell::column::data_type::bigint:
			{
				auto* i64 = static_cast<std::int64_t*>(value);
				dv.set_value(type, *i64);
				return new std::string(dv.to_string());
			}

		case cell::column::data_type::real:
			{
				auto* f32 = static_cast<float*>(value);
				dv.set_value(type, *f32);
				return new std::string(dv.to_string());
			}

		case cell::column::data_type::double_precision:
			{
				auto* f64 = static_cast<float*>(value);
				dv.set_value(type, *f64);
				return new std::string(dv.to_string());
			}

		case cell::column::data_type::varchar:
			return new std::string(*static_cast<std::string*>(value));
		}
	throw std::invalid_argument(
			"unknown value type when converting value to string.");

}
