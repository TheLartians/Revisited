#include <doctest/doctest.h>

#include <ostream>
#include <revisited/type_index.h>
#include <sstream>

TEST_CASE("Type Index") {

  using namespace revisited;

  REQUIRE(getTypeID<int>() == getTypeID<int>());
  REQUIRE(getTypeID<float>() == getTypeID<float>());
  REQUIRE(getTypeID<int>() != getTypeID<float>());

  using A = int;
  using B = float;

  REQUIRE(getTypeID<A>() == getTypeID<int>());
  REQUIRE(getTypeID<B>() == getTypeID<float>());
  REQUIRE(getTypeID<A>() != getTypeID<B>());
  REQUIRE(getTypeID<A>() != getTypeID<float>());
  REQUIRE(getTypeID<B>() != getTypeID<int>());

  REQUIRE(getTypeID<B>() != getTypeID<int>());
}

TEST_CASE("TypeID string conversions") {
  using namespace revisited;
  using A = int;
  using B = float;

  std::stringstream stream;

  SUBCASE("int") {
    stream << getTypeID<A>().name;
    REQUIRE(stream.str() == "int");
  }

  SUBCASE("float") {
    stream << getTypeID<B>().name;
    REQUIRE(stream.str() == "float");
  }
}
