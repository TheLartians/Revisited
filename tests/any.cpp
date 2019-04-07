#include <catch2/catch.hpp>
#include <lars/any.h>

TEST_CASE("Any") {
  using namespace lars;
  
  Any any;
  any.set<std::string>("Hello World!");

  REQUIRE(any.get<std::string>() == "Hello World!");
  REQUIRE_THROWS_AS(any.get<int>(), Any::BadAnyCast);
  
  any.set<int>(42);
  REQUIRE(any.get_numeric<int>() == 42);
  REQUIRE(any.get_numeric<unsigned>() == 42);
  REQUIRE(any.get_numeric<size_t>() == 42);
  REQUIRE(any.get_numeric<float>() == 42);
  REQUIRE(any.get_numeric<double>() == 42);

}
