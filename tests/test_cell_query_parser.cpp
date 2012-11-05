#include <memory>
#include <cell/cpp/query_parser.h>
#include <gtest/gtest.h>

class QueryParserTest: public ::testing::Test
{
public:
  lattice::cell::database *db;

  virtual void SetUp()
  {
    using namespace lattice::cell;

    db = new database();
    db->create_table("test_table_1",
          {
          new column
                {
                column::data_type::integer, "id", 4
                },
              new column
                {
                column::data_type::bigint, "c1", 8
                },
              new column
                {
                column::data_type::varchar, "c2", 0
                }
          });
  }

  virtual void TearDown()
  {
    delete db;
  }

};

TEST_F(QueryParserTest, CanCreate)
{
  using namespace lattice::cell;

  std::unique_ptr<query_parser> q;
  ASSERT_NO_THROW(q =std::unique_ptr<query_parser>(
      new query_parser(*db, std::string("test"))
      )
      );
}

TEST_F(QueryParserTest, CanParseSelectStar)
{
  using namespace lattice::cell;

  std::string query_data("select *");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());
}

TEST_F(QueryParserTest, CanParseSelectOne)
{
  using namespace lattice::cell;

  std::string query_data("select 1");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  ASSERT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectOneCommaTwo)
{
  using namespace lattice::cell;

  std::string query_data("select 1, 2");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect two select expressions.
   auto& se = qp.get_query().get_select_expressions();
   EXPECT_EQ(2, se.size());

   EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
   EXPECT_EQ(actions::node::node_type::LITERAL, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectColumns)
{
  using namespace lattice::cell;

  std::string query_data("select c1, c2");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect two select expressions.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(2, se.size());

  EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
  EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectColumnAndNumber)
{
  using namespace lattice::cell;

  std::string query_data("select 1, c2");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect two select expressions.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(2, se.size());

  EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());
  EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[1]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectString)
{
  using namespace lattice::cell;

  std::string query_data("select 'test value'");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  /*auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::LITERAL, se[0]->get_type());*/
}

TEST_F(QueryParserTest, CanParseAddition)
{
  using namespace lattice::cell;

  std::string query_data("select 7+9");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::OP_ADD, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSubtraction)
{
  using namespace lattice::cell;

  std::string query_data("select 9-7");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::OP_SUB, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseMultiplication)
{
  using namespace lattice::cell;

  std::string query_data("select 8*8");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::OP_MUL, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseDivision)
{
  using namespace lattice::cell;

  std::string query_data("select 16/4");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::OP_DIV, se[0]->get_type());
}

TEST_F(QueryParserTest, CanParseSelectFrom)
{
  using namespace lattice::cell;

  std::string query_data("select c1 from test_table_1");
  query_parser qp(*db, query_data);

  EXPECT_TRUE(qp.parse());

  // Expect one select expression.
  auto& se = qp.get_query().get_select_expressions();
  EXPECT_EQ(1, se.size());

  EXPECT_EQ(actions::node::node_type::COLUMN_REF, se[0]->get_type());
}




