#include <catch2/catch.hpp>

#include <lars/any_function.h>

using namespace lars;

TEST_CASE("call without arguments","[any_function]"){
  AnyFunction f;
  REQUIRE(bool(f) == false);
  REQUIRE_THROWS_AS(f(), AnyFunctionUndefinedCallException);
  REQUIRE_THROWS_WITH(f(), Catch::Matchers::Contains("undefined AnyFunction"));

  SECTION("no return value"){
    auto value = 0;
    f = [&](){ value = 42; };
    REQUIRE_NOTHROW(f());
    REQUIRE(value == 42);
  }
  
  SECTION("return value"){
    f = [](){ return 42; };
    REQUIRE(f().get<int>() == 42);
  }
  
  REQUIRE(bool(f) == true);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_WITH(f(1), Catch::Matchers::Contains("wrong number of arguments"));
}

TEST_CASE("call with arguments","[any_function]"){
  AnyFunction f = [](int a, double b){ return a + b; };
  REQUIRE(f(1,2).get<int>() == 3);
  REQUIRE(f(1,2.5).get<double>() == 3.5);
  REQUIRE_THROWS_AS(f(), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1,2,3), AnyFunctionInvalidArgumentCountException);
}
  
TEST_CASE("call with any arguments","[any_function]"){
  AnyFunction f = [](AnyArguments &args){
    double result = 0;
    for(auto &arg: args) { result += arg.get<double>(); }
    return result;
  };
  REQUIRE(f().get<double>() == 0);
  REQUIRE(f(1, 2).get<double>() == 3);
  REQUIRE(f(1, 2, 3, 4, 5).get<double>() == 15);
}

