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

