#include <catch2/catch.hpp>
#include <lars/any.h>

TEST_CASE("Any") {
  using namespace lars;
  
  Any any;
  any.set<std::string>("Hello World!");

  REQUIRE(any.get<std::string>() == "Hello World!");
  REQUIRE_THROWS_AS(any.get<int>(), Any::BadAnyCast);
  REQUIRE_THROWS_AS(any.get_numeric<int>(), Any::BadAnyCast);
  
  any.set<int>(42);
  REQUIRE(any.get_numeric<int>() == 42);
  REQUIRE(any.get_numeric<unsigned>() == 42);
  REQUIRE(any.get_numeric<size_t>() == 42);
  REQUIRE(any.get_numeric<float>() == 42);
  REQUIRE(any.get_numeric<double>() == 42);
  REQUIRE_THROWS_AS(any.get<std::string>(), Any::BadAnyCast);

}

TEST_CASE("AnyFunction") {
  using namespace lars;
  
  AnyFunction get = [](){ return 42; };
  REQUIRE(get().get<int>() == 42);
  
  AnyFunction getAny = [](){ return make_any<int>(42); };
  REQUIRE(getAny().get<int>() == 42);

  AnyFunction take = [](float x, float y){ return x+y; };
  REQUIRE_THROWS(take());
  REQUIRE_THROWS(take(2));
  REQUIRE(take(2,3).get_numeric() == Approx(5));
  REQUIRE(take(2,3.5).get_numeric() == Approx(5.5));
  REQUIRE_THROWS(take(2,3,5));
  
  AnyFunction takeAny = [](Any x, Any y){ return x.get_numeric()+y.get_numeric(); };
  REQUIRE(takeAny(2,3).get_numeric() == Approx(5));
  REQUIRE(takeAny(take(2,3.5),3).get_numeric() == Approx(8.5));

}
