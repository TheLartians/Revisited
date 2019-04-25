#include <catch2/catch.hpp>
#include <lars/any.h>

namespace{
  struct A{ };
  struct B:public A{ };
  struct C:public B{ };
  struct D{ };
  struct E: public C, public D{ };
}

TEST_CASE("any"){
  
}

TEMPLATE_TEST_CASE("numeric any", "", char, int, long, long long, unsigned char, unsigned int, unsigned long, unsigned long long, float, double) {
  lars::Any v;

  v.set<TestType>(42);

  REQUIRE(v.get<TestType &>() == 42);
  REQUIRE(v.get<const TestType &>() == 42);
  REQUIRE(v.get<char>() == 42);
  REQUIRE(v.get<int>() == 42);
  REQUIRE(v.get<unsigned>() == 42);
  REQUIRE(v.get<long>() == 42);
  REQUIRE(v.get<float>() == 42);
  REQUIRE(v.get<double>() == 42);
  REQUIRE(v.get<size_t>() == 42);
}

TEST_CASE("string any"){
  lars::Any v;
  
  SECTION("string"){
    v = std::string("Hello Any!");
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }

  SECTION("string literal"){
    v = "Hello Any!";
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }
  
}
