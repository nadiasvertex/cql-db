#include <cstdint>
#include <memory>
#include <sstream>

#include <processor/cpp/row_buffer.h>

#include <gtest/gtest.h>

TEST(RowBuffer, CanCreate)
{
	using namespace lattice::processor;

	std::unique_ptr<row_buffer> buf;

	ASSERT_NO_THROW(buf = std::unique_ptr<row_buffer>(new row_buffer()));
}
