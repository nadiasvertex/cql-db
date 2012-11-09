#include <cstdint>
#include <memory>
#include <sstream>
#include <vector>

#include <processor/cpp/metadata.h>

#include <gtest/gtest.h>

class MetadataTest: public ::testing::Test
{
public:
  lattice::processor::metadata *md;

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
                },
              column
                {
                column::data_type::bigint, "c1", 8
                },
              column
                {
                column::data_type::varchar, "c2", 0
                }
          });
  }

  virtual void TearDown()
  {
    delete md;
  }

};

TEST_F(MetadataTest, CanCreate)
{
  using namespace lattice::processor;

  std::unique_ptr<metadata> q;
  ASSERT_NO_THROW(q =std::unique_ptr<metadata>(
      new metadata()
      )
      );
}

TEST_F(MetadataTest, CanFindTable)
{
  using namespace lattice::processor;

  EXPECT_TRUE(md->has_table("test_table_1"));
}

TEST_F(MetadataTest, CanGetColumnType)
{
  using namespace lattice::processor;

  auto result = md->get_column_type("test_table_1", "c1");

  EXPECT_TRUE(std::get<1>(result));
}
