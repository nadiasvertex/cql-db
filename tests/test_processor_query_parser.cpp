#include <memory>
#include <processor/cpp/query_parser.h>
#include <gtest/gtest.h>

class QueryParserTest: public ::testing::Test
{
public:
	lattice::processor::metadata *md;

	virtual void SetUp()
	{
		using namespace lattice;

		md = new processor::metadata();
		md->create_table("test_table_1",
			{
			cell::column
				{
					cell::column::data_type::integer, "id", 4
				},
			cell::column
				{
					cell::column::data_type::bigint, "c1", 8
				},
			cell::column
				{
					cell::column::data_type::varchar, "c2", 0
				}
			});
	}

	virtual void TearDown()
	{
		delete md;
	}

};

TEST_F(QueryParserTest, CanCreate)
{
	using namespace lattice::processor;

	std::unique_ptr < query_parser > q;
	ASSERT_NO_THROW(
			q =std::unique_ptr<query_parser>( new query_parser(*md, std::string("test")) ));
}

TEST_F(QueryParserTest, CanParseSelectStar)
{
	using namespace lattice::processor;

	std::string query_data("select *");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());
}

TEST_F(QueryParserTest, CanParseSelectOne)
{
	using namespace lattice::processor;

	std::string query_data("select 1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	ASSERT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectOneCommaTwo)
{
	using namespace lattice::processor;

	std::string query_data("select 1, 2");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(2, se.size());

	EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
	EXPECT_EQ(actions::node::node_type::LITERAL, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectColumns)
{
	using namespace lattice::processor;

	std::string query_data("select c1, c2");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(2, se.size());

	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectTableDotColumn)
{
	using namespace lattice::processor;

	std::string query_data("select t1.c1, t1.c2");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(2, se.size());

	EXPECT_EQ(actions::node::node_type::TABLE_REF, se[0]->get_type());
	EXPECT_EQ(actions::node::node_type::TABLE_REF, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectColumnAndNumber)
{
	using namespace lattice::processor;

	std::string query_data("select 1, c2");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(2, se.size());

	EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectString)
{
	using namespace lattice::processor;

	std::string query_data("select 'test value'");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseAddition)
{
	using namespace lattice::processor;

	std::string query_data("select 7+9");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::OP_ADD, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSubtraction)
{
	using namespace lattice::processor;

	std::string query_data("select 9-7");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::OP_SUB, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseMultiplication)
{
	using namespace lattice::processor;

	std::string query_data("select 8*8");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::OP_MUL, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseDivision)
{
	using namespace lattice::processor;

	std::string query_data("select 16/4");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::OP_DIV, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectFrom)
{
	using namespace lattice::processor;

	std::string query_data("select c1 from test_table_1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectTwoColumnsFrom)
{
	using namespace lattice::processor;

	std::string query_data("select c1+10, id*10 from test_table_1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(2, se.size());

	EXPECT_EQ(actions::node::node_type::OP_ADD, se[0]->get_type());
	EXPECT_EQ(actions::node::node_type::OP_MUL, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSimpleInnerJoin)
{
	using namespace lattice::processor;

	std::string query_data(
			"select c1 from test_table_1 as t1 inner join test_table_2 as t2 on t1.c1=t2.c1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseLeftOuterJoin)
{
	using namespace lattice::processor;

	std::string query_data(
			"select c1 from test_table_1 as t1 left outer join test_table_2 as t2 on t1.c1=t2.c1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseRightOuterJoin)
{
	using namespace lattice::processor;

	std::string query_data(
			"select c1 from test_table_1 as t1 right outer join test_table_2 as t2 on t1.c1=t2.c1");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect one select expression.
	auto& se = qp.get_query().get_select_expressions();
	EXPECT_EQ(1, se.size());

	EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
}

TEST_F(QueryParserTest, CanGetColumnVector)
{
	using namespace lattice::processor;

	std::string query_data("select test2.c3, c1, c2, c3");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto cv = qp.get_query().get_column_vector();
	ASSERT_EQ(4, cv.size());

	EXPECT_EQ("test2.c3", cv[0]);
	EXPECT_EQ("c1", cv[1]);
	EXPECT_EQ("c2", cv[2]);
	EXPECT_EQ("c3", cv[3]);

}

TEST_F(QueryParserTest, CanGetColumnVector2)
{
	using namespace lattice::processor;

	std::string query_data("select c2+c3, c1, c2, c4, c3");
	query_parser qp(*md, query_data);

	EXPECT_TRUE(qp.parse());

	// Expect two select expressions.
	auto cv = qp.get_query().get_column_vector();
	ASSERT_EQ(4, cv.size());

	EXPECT_EQ("c2", cv[0]);
	EXPECT_EQ("c3", cv[1]);
	EXPECT_EQ("c1", cv[2]);
	EXPECT_EQ("c4", cv[3]);
}
