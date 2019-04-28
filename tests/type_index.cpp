#include <catch2/catch.hpp>
#include <lars/to_string.h>

#include <lars/type_index.h>


TEST_CASE("Type Index") {

  using namespace lars;

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
  
  REQUIRE(lars::stream_to_string(getTypeIndex<A>()) == "int");
  REQUIRE(lars::stream_to_string(getTypeIndex<B>()) == "float");

}
