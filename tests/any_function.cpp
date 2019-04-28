#include <catch2/catch.hpp>

#include <lars/any_function.h>
#include <lars/log.h>
using namespace lars;

TEST_CASE("call without arguments","[any_function]"){
  AnyFunction f;
  REQUIRE(bool(f) == false);

  REQUIRE_THROWS_AS(f(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.returnType(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.argumentCount(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.argumentType(0), UndefinedAnyFunctionException);
  REQUIRE_THROWS_WITH(f(), Catch::Matchers::Contains("undefined AnyFunction"));

  SECTION("no return value"){
    auto value = 0;
    f = [&](){ value = 42; };
    
    REQUIRE(f.returnType() == getStaticTypeIndex<void>());
    REQUIRE(f.argumentCount() == 0);
    REQUIRE(f.argumentType(0) == getStaticTypeIndex<void>());
    REQUIRE(!f.isVariadic());

    REQUIRE_NOTHROW(f());
    REQUIRE(value == 42);
  }
  
  SECTION("return value"){
    f = []()->int{ return 42; };
    
    REQUIRE(f.returnType() == getStaticTypeIndex<int>());
    REQUIRE(f.argumentCount() == 0);
    REQUIRE(f.argumentType(0) == getStaticTypeIndex<void>());
    REQUIRE(!f.isVariadic());

    REQUIRE(f().get<int>() == 42);
  }
  
  REQUIRE(bool(f) == true);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_WITH(f(1), Catch::Matchers::Contains("wrong number of arguments"));
}

TEST_CASE("call with arguments","[any_function]"){
  AnyFunction f = [](int a, double b){ return a - b; };
  REQUIRE(f.returnType() == getStaticTypeIndex<double>());
  REQUIRE(f.argumentCount() == 2);
  REQUIRE(f.argumentType(0) == getStaticTypeIndex<int>());
  REQUIRE(f.argumentType(1) == getStaticTypeIndex<double>());
  REQUIRE(!f.isVariadic());

  REQUIRE(f(1,2).type() == getStaticTypeIndex<double>());
  REQUIRE(f(1,2).get<int>() == -1);
  REQUIRE(f(2,1).get<int>() == 1);
  REQUIRE(f(1.5,1).get<double>() == 0);
  REQUIRE(f(1,1.5).get<double>() == -0.5);
  
  REQUIRE_THROWS_AS(f(), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1,2,3), AnyFunctionInvalidArgumentCountException);
}

TEST_CASE("call with reference arguments","[any_function]"){
  AnyFunction f = [](int &x){ x++; };
  int x = 41;
  f(x);
  REQUIRE(x == 42);
}

TEST_CASE("return any","[any_function]"){
  AnyFunction f = [](){ return Any(42); };
  REQUIRE(f().get<int>() == 42);
}

TEST_CASE("pass any","[any_function]"){
  AnyFunction f = [](int x){ REQUIRE(x == 42); };
  Any x = 42;
  f(x);
}

TEST_CASE("take any","[any_function]"){
  AnyFunction f = [](const Any &x){ REQUIRE(x.get<int>() == 42); };
  f(42);
}

TEST_CASE("call with any arguments","[any_function]"){
  AnyFunction f = [](const AnyArguments &args){
    double result = 0;
    for(auto &arg: args) { result += arg.get<double>(); }
    return result;
  };

  REQUIRE(f.returnType() == getStaticTypeIndex<double>());
  REQUIRE(f.isVariadic());
  REQUIRE(f.argumentCount() == 0);
  REQUIRE(f.argumentType(42) == getStaticTypeIndex<Any>());

  REQUIRE(f().get<int>() == 0);
  REQUIRE(f(1, 2).get<float>() == 3);
  REQUIRE(f(1, 2, 3, 4, 5).get<unsigned>() == 15);
}

TEST_CASE("implicit_string_conversion","[any_function]"){
  AnyFunction f = [](std::string v)->std::string{ return "Hello " + v + "!"; };
  REQUIRE(f("AnyFunction").get<std::string>() == "Hello AnyFunction!");
}

TEST_CASE("any_function_with_std_function","[any_function][any]"){
  AnyFunction f = std::function<int()>([](){ return 42; });
  REQUIRE(f().get<int>() == 42);
}

TEST_CASE("any_with_any_function","[any_function][any]"){
  AnyFunction f = [](){ return 42; };
  Any af = f;
  REQUIRE(af.get<const AnyFunction &>()().get<int>() == 42);
}
