#include <catch2/catch.hpp>
#include <lars/to_string.h>

#include <lars/any.h>

using namespace lars;

TEST_CASE("Get", "[any]"){
  Any v;
  REQUIRE(v.type() == getStaticTypeIndex<void>());

  REQUIRE(bool(v) == false);
  REQUIRE_THROWS_AS(v.get<int>(), UndefinedAnyException);
  REQUIRE_THROWS_WITH(v.get<int>(), Catch::Matchers::Contains("undefined Any"));
  v.set<int>(3);
  REQUIRE(v.type() == getStaticTypeIndex<int>());
  REQUIRE(stream_to_string(v) == "Any<int>");

  REQUIRE(bool(v) == true);
  REQUIRE(v.get<int>() == 3);
  REQUIRE(std::as_const(v).get<int>() == 3);
  REQUIRE(v.get<int &>() == 3);
  REQUIRE(v.get<const int &>() == 3);
  REQUIRE(v.tryGet<int>()== &v.get<int &>());
  REQUIRE_THROWS_AS(v.get<std::string>(), InvalidVisitorException);
  REQUIRE(v.tryGet<std::string>() == nullptr);
  
  v = "Hello any!";
  REQUIRE(v.get<std::string>() == "Hello any!");
  REQUIRE(v.get<std::string &>() == "Hello any!");
  REQUIRE(v.get<const std::string &>() == "Hello any!");
  REQUIRE(v.tryGet<std::string>()== &v.get<std::string &>());
  REQUIRE_THROWS_AS(v.get<int>(), InvalidVisitorException);
  REQUIRE(v.tryGet<int>() == nullptr);
}

TEMPLATE_TEST_CASE("Numerics", "[any]", char, int, long, long long, unsigned char, unsigned int, unsigned long, unsigned long long, float, double) {
  Any v;

  v.set<TestType>(42);

  REQUIRE(v.type() == getStaticTypeIndex<TestType>());
  
  REQUIRE(v.get<TestType &>() == 42);
  REQUIRE(v.get<const TestType &>() == 42);
  REQUIRE(v.get<char>() == 42);
  REQUIRE(v.get<int>() == 42);
  REQUIRE(v.get<unsigned>() == 42);
  REQUIRE(v.get<long>() == 42);
  REQUIRE(v.get<float>() == 42);
  REQUIRE(v.get<double>() == 42);
  REQUIRE(v.get<size_t>() == 42);
  REQUIRE_THROWS_AS(v.get<std::string>(), InvalidVisitorException);
}

TEST_CASE("floating point conversions","[any]"){
  Any v = 1.5;
  REQUIRE(v.get<double>() == 1.5);
  REQUIRE(v.get<int>() == 1);
  v = 3.141;
  REQUIRE(v.get<float>() == Approx(3.141));
}

TEST_CASE("String", "[any]"){
  Any v;
  REQUIRE_THROWS_AS(v.get<std::string>(), UndefinedAnyException);

  SECTION("string"){
    v = std::string("Hello Any!");
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }

  SECTION("string literal"){
    v = "Hello Any!";
    REQUIRE(v.get<std::string>() == "Hello Any!");
  }
  
  REQUIRE_THROWS_AS(v.get<int>(), InvalidVisitorException);
}

TEST_CASE("Inheritance", "[any]"){
  struct A{ char name = 'A'; };
  struct B:public A{ char name = 'B'; };
  struct C:public B{ char name = 'C'; };
  struct D{ char name = 'D'; };
  struct E: public C, public D{ char name = 'E'; };
  
  SECTION("Custom class"){
    Any v = A();
    REQUIRE(v.type() == getStaticTypeIndex<A>());
    REQUIRE(v.get<A>().name == 'A');
    REQUIRE(v.get<A &>().name == 'A');
    REQUIRE(v.get<const A &>().name == 'A');
    REQUIRE_THROWS_AS(v.get<B>(), InvalidVisitorException);
  }
  
  SECTION("Inheritance"){
    Any v;
    v.setWithBases<E,D,C,B,A>();
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
  struct D: public DerivedVisitable<D,VirtualVisitable<A, B>> { char name = 'D'; };
  struct E: public DerivedVisitable<E,VirtualVisitable<D, A>> { char name = 'E'; };
  auto v = make_any<E>();
  REQUIRE(v.get<A &>().name == 'A');
  REQUIRE(v.get<const B &>().name == 'B');
  REQUIRE_THROWS_AS(v.get<C &>(), InvalidVisitorException);
  REQUIRE(v.get<D &>().name == 'D');
  REQUIRE(v.get<const E &>().name == 'E');
}

TEST_CASE("capture reference","[any]"){
  int x = 1;
  Any y = std::reference_wrapper<int>(x);
  REQUIRE(&y.get<int &>() == &x);
  REQUIRE(&y.get<const int &>() == &x);
  y.get<int &>() = 2;
  REQUIRE(x == 2);
}

TEST_CASE("AnyReference","[any]"){
  Any x = 1;
  AnyReference y = x;
  y.get<int &>() = 2;
  REQUIRE(x.get<int>() == 2);
}
