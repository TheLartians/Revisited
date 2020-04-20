#include <doctest/doctest.h>

#include <revisited/type_index.h>
#include <sstream>

TEST_CASE("Type Index") {

  using namespace revisited;

  REQUIRE(getStaticTypeIndex<int>() == getStaticTypeIndex<int>());
  REQUIRE(getStaticTypeIndex<float>() == getStaticTypeIndex<float>());
  REQUIRE(getStaticTypeIndex<int>() != getStaticTypeIndex<float>());

  using A = int;
  using B = float;

  REQUIRE(getStaticTypeIndex<A>() == getStaticTypeIndex<int>());
  REQUIRE(getStaticTypeIndex<B>() == getStaticTypeIndex<float>());
  REQUIRE(getStaticTypeIndex<A>() != getStaticTypeIndex<B>());
  REQUIRE(getStaticTypeIndex<A>() != getStaticTypeIndex<float>());
  REQUIRE(getStaticTypeIndex<B>() != getStaticTypeIndex<int>());

  REQUIRE(getStaticTypeIndex<B>() != getStaticTypeIndex<int>());
  

}

TEST_CASE("TypeIndex string conversions") {
   using namespace revisited;
 using A = int;
  using B = float;

  std::stringstream stream;

  SUBCASE("int") {
    stream << getTypeIndex<A>();
    REQUIRE(stream.str() == "int");
  }

  SUBCASE("float") {
    stream << getTypeIndex<B>();
    REQUIRE(stream.str() == "float");
  }
  
}

