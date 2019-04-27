#include <catch2/catch.hpp>
#include <lars/to_string.h>

#include <lars/type_index.h>


TEST_CASE("Type Index") {

  using namespace lars;

  REQUIRE(getTypeIndex<int>() == getTypeIndex<int>());
  REQUIRE(getTypeIndex<float>() == getTypeIndex<float>());
  REQUIRE(getTypeIndex<int>() != getTypeIndex<float>());

  using A = int;
  using B = float;

  REQUIRE(getTypeIndex<A>() == getTypeIndex<int>());
  REQUIRE(getTypeIndex<B>() == getTypeIndex<float>());
  REQUIRE(getTypeIndex<A>() != getTypeIndex<B>());
  REQUIRE(getTypeIndex<A>() != getTypeIndex<float>());
  REQUIRE(getTypeIndex<B>() != getTypeIndex<int>());

  REQUIRE(getTypeIndex<B>() != getTypeIndex<int>());
  
  REQUIRE(lars::stream_to_string(getNamedTypeIndex<A>()) == "int");
  REQUIRE(lars::stream_to_string(getNamedTypeIndex<B>()) == "float");

}
