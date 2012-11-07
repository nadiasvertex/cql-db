#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <random>

#include <processor/cpp/row_buffer.h>

#include <gtest/gtest.h>

class RowBufferTest: public ::testing::Test
{
public:
	std::default_random_engine dre;
	std::uniform_int_distribution<std::uint32_t> di;

	lattice::processor::Row row_data;

	lattice::processor::row_buffer::row_header_type columns;

	std::uint64_t id_next;

	virtual void SetUp()
	{
		using namespace lattice::cell;
		using namespace lattice::processor;

		columns.push_back(lattice::cell::column
			{
			lattice::cell::column::data_type::bigint, "id", 8
			});
		columns.push_back(lattice::cell::column
			{
			lattice::cell::column::data_type::integer, "c1", 4
			});
		columns.push_back(lattice::cell::column
			{
			lattice::cell::column::data_type::varchar, "c2", 0
			});

		id_next = 0;
	}

	virtual void TearDown()
	{

	}

	void FillRowData()
	{
		using namespace lattice::cell;

		std::string s1(" a fragment ");
		std::string s2(" other fragment ");

		data_value d1, d2, d3;

		d1.set_value(column::data_type::bigint, id_next++);
		d2.set_value(column::data_type::integer, di(dre));

		std::string v;

		auto limit = di(dre) % 5;
		for (auto i = 0; i < limit; ++i)
			{
				if (di(dre) & 1)
					{
						v.append(s1);
					}
				else
					{
						v.append(s2);
					}
			}

		d3.set_value(column::data_type::varchar, v);

		std::stringstream out;

		d1.write(out);
		d2.write(out);
		d3.write(out);

		auto data = out.str();
		auto size = data.size();

		row_data.set_id(id_next);
		row_data.set_data(data);
	}

};

TEST_F(RowBufferTest, CanCreate)
{
	using namespace lattice::processor;

	std::unique_ptr<row_buffer> buf;

	ASSERT_NO_THROW(buf = std::unique_ptr<row_buffer>(new row_buffer(columns)));
}

TEST_F(RowBufferTest, CanEnqueue)
{
	using namespace lattice::processor;

	row_buffer rb(columns);

	FillRowData();

	ASSERT_NO_THROW(rb.enqueue(row_data));
}

TEST_F(RowBufferTest, CanDequeue)
{
	using namespace lattice::processor;

	row_buffer rb(columns);

	FillRowData();

	rb.enqueue(row_data);

	ASSERT_NO_THROW(rb.dequeue());
}

TEST_F(RowBufferTest, CanGetCurrentRow)
{
	using namespace lattice::processor;

	row_buffer rb(columns);

	FillRowData();

	rb.enqueue(row_data);
	rb.dequeue();

	ASSERT_EQ(3, rb.get_current_row().size());
}

