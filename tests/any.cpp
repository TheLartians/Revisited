#include <catch2/catch.hpp>
#include <lars/any.h>

using namespace lars;

TEMPLATE_TEST_CASE("Numerics", "[any]", char, int, long, long long, unsigned char, unsigned int, unsigned long, unsigned long long, float, double) {
  Any v;

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

TEST_CASE("String", "[any]"){
  
  SECTION("string"){
    Any v = std::string("Hello Any!");
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }

  SECTION("string literal"){
    Any v = "Hello Any!";
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }
}

TEST_CASE("Inheritance", "[any]"){
  struct A{ char name = 'A'; };
  struct B:public A{ char name = 'B'; };
  struct C:public B{ char name = 'C'; };
  struct D{ char name = 'D'; };
  struct E: public C, public D{ char name = 'E'; };
  
  SECTION("Custom class"){
    Any v = A();
    REQUIRE(v.get<A>().name == 'A');
    REQUIRE(v.get<A &>().name == 'A');
    REQUIRE(v.get<const A &>().name == 'A');
    REQUIRE_THROWS_AS(v.get<B>(), InvalidVisitorException);
  }
  
  SECTION("Inheritance"){
    Any v;
    v.set<E, DataVisitableWithBases<E,D,C,B,A>>();
    REQUIRE(v.get<A &>().name == 'A');
    REQUIRE(v.get<const B &>().name == 'B');
    REQUIRE(v.get<C>().name == 'C');
    REQUIRE(v.get<D &>().name == 'D');
    REQUIRE(v.get<const E &>().name == 'E');
  }
}

TEST_CASE("Visitable inheritance","[any]"){
  struct A: Visitable<A> { char name = 'A'; };
  struct B: Visitable<B> { char name = 'B'; };
  struct C: public DerivedVisitable<C, A> { char name = 'C'; };
  struct D: public DerivedVisitable<D,VirtualJoinVisitable<A, B>> { char name = 'D'; };
  struct E: public DerivedVisitable<E,VirtualJoinVisitable<D, A>> { char name = 'E'; };
  Any v = E();
  REQUIRE(v.get<A &>().name == 'A');
  REQUIRE(v.get<const B &>().name == 'B');
  REQUIRE_THROWS_AS(v.get<C &>(), InvalidVisitorException);
  REQUIRE(v.get<D &>().name == 'D');
  REQUIRE(v.get<const E &>().name == 'E');
}
