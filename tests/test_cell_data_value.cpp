#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/data_value.h>

#include <gtest/gtest.h>

TEST(DataValueTest, CanCreate)
{
  using namespace lattice::cell;

  std::unique_ptr<data_value> dv;
  ASSERT_NO_THROW(
      dv = std::unique_ptr<data_value>(new data_value())
      );
}

TEST(DataValueTest, SmallInt)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  dv1.set_value(column::data_type::smallint, 10);
  dv2.set_value(column::data_type::smallint, 10);

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);
}

TEST(DataValueTest, Int)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  dv1.set_value(column::data_type::integer, 100000);
  dv2.set_value(column::data_type::integer, 100000);

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);
}

TEST(DataValueTest, BigInt)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  std::uint64_t value = 10LL << 32;

  dv1.set_value(column::data_type::bigint, value);
  dv2.set_value(column::data_type::bigint, value);

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);
}

TEST(DataValueTest, Real)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  dv1.set_value(column::data_type::real, 10.5);
  dv2.set_value(column::data_type::real, 10.5);

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);
}

TEST(DataValueTest, DoublePrecision)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  dv1.set_value(column::data_type::double_precision, 10.5);
  dv2.set_value(column::data_type::double_precision, 10.5);

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);

  data_value dv3, dv4;

  dv3.set_value(column::data_type::double_precision, 101.5);
  dv4.set_value(column::data_type::double_precision, 100043.2);

  ASSERT_FALSE(dv3 == dv4);
  ASSERT_FALSE(dv4 == dv3);
  ASSERT_TRUE(dv3 < dv4);
  ASSERT_FALSE(dv4 < dv3);
}

TEST(DataValueTest, VarChar)
{
  using namespace lattice::cell;

  data_value dv1, dv2;

  dv1.set_value(column::data_type::varchar, std::string("test"));
  dv2.set_value(column::data_type::varchar, std::string("test"));

  ASSERT_TRUE(dv1 == dv2);
  ASSERT_FALSE(dv1 < dv2);

  data_value dv3, dv4;

  dv3.set_value(column::data_type::varchar, std::string("abc"));
  dv4.set_value(column::data_type::varchar, std::string("xyz"));

  ASSERT_FALSE(dv3 == dv4);
  ASSERT_TRUE(dv3 < dv4);
  ASSERT_FALSE(dv4 < dv3);
}

