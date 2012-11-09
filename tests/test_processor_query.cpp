#include <cstdint>
#include <memory>
#include <sstream>

#include <processor/cpp/query.h>
#include <processor/cpp/row_buffer.h>

#include <gtest/gtest.h>

class QueryTest: public ::testing::Test
{
public:
	lattice::processor::metadata *md;
	lattice::processor::row_buffer *stock_rb;

	virtual void SetUp()
	{
		using namespace lattice::cell;
		using namespace lattice::processor;

		md = new metadata();
		md->create_table("test_table_1",
			{
			column
				{
				column::data_type::integer, "id", 4
				}, column
				{
				column::data_type::bigint, "c1", 8
				}, column
				{
				column::data_type::varchar, "c2", 0
				}
			});

		row_buffer::row_header_type rh
			{
			column
				{
				column::data_type::integer, "id", 4
				}, column
				{
				column::data_type::bigint, "c1", 8
				}, column
				{
				column::data_type::varchar, "c2", 0
				}
			};

		stock_rb = new row_buffer
			{
			rh
			};
	}

	virtual void TearDown()
	{
		delete stock_rb;
		delete md;
	}

	void GenerateRowData(lattice::processor::row_buffer& rb)
	{
		using namespace lattice::cell;

		data_value d1, d2, d3;
		lattice::processor::Row row_data;

		d1.set_value(column::data_type::integer, 1);
		d2.set_value(column::data_type::bigint, 10000);
		d3.set_value(column::data_type::varchar, std::string("This is a test."));

		std::stringstream out;

		d1.write(out);
		d2.write(out);
		d3.write(out);

		auto data = out.str();
		auto size = data.size();

		row_data.set_id(1);
		row_data.set_data(data);

		rb.enqueue(row_data);
		rb.dequeue();
	}

};

TEST_F(QueryTest, CanCreate)
{
	using namespace lattice::processor;

	std::unique_ptr<query> q;
	ASSERT_NO_THROW(q =std::unique_ptr<query>( new query(*md, "") ));
}

TEST_F(QueryTest, CanSelectOne)
{
	using namespace lattice::processor;

	query q(*md, "select 1");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("1"), r[0]);
}

TEST_F(QueryTest, CanSelectOnePlusOne)
{
	using namespace lattice::processor;

	query q(*md, "select 1+1");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("2"), r[0]);
}

TEST_F(QueryTest, CanSelectComplexAddition)
{
	using namespace lattice::processor;

	query q(*md, "select 1+2+3+4+5+6+7+8+9+10");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("55"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumnAddition)
{
	using namespace lattice::processor;

	query q(*md, "select 1+2+3+4+5+6+7+8+9+10, 20+30+40+50");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("55"), r[0]);
	EXPECT_EQ(std::string("140"), r[1]);
}

TEST_F(QueryTest, CanSelectTenTimesTen)
{
	using namespace lattice::processor;

	query q(*md, "select 10*10");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("100"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumnMultiplication)
{
	using namespace lattice::processor;

	query q(*md, "select 10*10*10, 2*4*8");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("1000"), r[0]);
	EXPECT_EQ(std::string("64"), r[1]);
}

TEST_F(QueryTest, CanSelectMultiColumnDivision)
{
	using namespace lattice::processor;

	query q(*md, "select 100/10, 16/4/2");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("10"), r[0]);
	EXPECT_EQ(std::string("2"), r[1]);
}

TEST_F(QueryTest, CanSelectMultiColumnSubtraction)
{
	using namespace lattice::processor;

	query q(*md, "select 25-5, 17-7");

	auto r = q.fetch_one(*stock_rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("20"), r[0]);
	EXPECT_EQ(std::string("10"), r[1]);
}

TEST_F(QueryTest, CanSelectColumn)
{
	using namespace lattice::processor;
	using namespace lattice::cell;

	query q(*md, "select id from test_table_1");

	row_buffer::row_header_type rh
		{
		column
			{
			column::data_type::integer, "id", 4
			}
		};

	lattice::processor::row_buffer rb
		{
		rh
		};

	GenerateRowData(rb);

	auto r = q.fetch_one(rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("1"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumn)
{
	using namespace lattice::processor;
	using namespace lattice::cell;

	query q(*md, "select id, c1 from test_table_1");

	row_buffer::row_header_type rh
		{
		column
			{
			column::data_type::integer, "id", 4
			}, column
			{
			column::data_type::bigint, "c1", 8
			}
		};

	lattice::processor::row_buffer rb
		{
		rh
		};

	GenerateRowData(rb);

	auto r = q.fetch_one(rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("1"), r[0]);
	EXPECT_EQ(std::string("10000"), r[1]);
}

TEST_F(QueryTest, CanSelectColumnWithLiteralExpression)
{
	using namespace lattice::processor;
	using namespace lattice::cell;

	query q(*md, "select id+10 from test_table_1");

	row_buffer::row_header_type rh
		{
		column
			{
			column::data_type::integer, "id", 4
			}
		};

	lattice::processor::row_buffer rb
		{
		rh
		};

	GenerateRowData(rb);

	auto r = q.fetch_one(rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("11"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumnWithLiteralExpression)
{
	using namespace lattice::processor;
	using namespace lattice::cell;

	query q(*md, "select id+10, c1/2 from test_table_1");

	row_buffer::row_header_type rh
		{
		column
			{
			column::data_type::integer, "id", 4
			}, column
			{
			column::data_type::bigint, "c1", 8
			}
		};

	lattice::processor::row_buffer rb
		{
		rh
		};

	GenerateRowData(rb);

	auto r = q.fetch_one(rb);

	ASSERT_EQ(2, r.size());
	EXPECT_EQ(std::string("11"), r[0]);
	EXPECT_EQ(std::string("5000"), r[1]);
}

TEST_F(QueryTest, CanSelectAddColumns)
{
	using namespace lattice::processor;
	using namespace lattice::cell;

	query q(*md, "select id+c1 from test_table_1");

	row_buffer::row_header_type rh
		{
		column
			{
			column::data_type::integer, "id", 4
			}, column
			{
			column::data_type::bigint, "c1", 8
			}
		};

	lattice::processor::row_buffer rb
		{
		rh
		};

	GenerateRowData(rb);

	auto r = q.fetch_one(rb);

	ASSERT_EQ(1, r.size());
	EXPECT_EQ(std::string("10001"), r[0]);
}


