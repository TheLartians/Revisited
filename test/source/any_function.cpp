#include <doctest/doctest.h>

#include <revisited/any_function.h>

using namespace revisited;

TEST_CASE("call without arguments") {
  AnyFunction f;
  REQUIRE(bool(f) == false);

  REQUIRE_THROWS_AS(f(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.returnType(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.argumentCount(), UndefinedAnyFunctionException);
  REQUIRE_THROWS_AS(f.argumentType(0), UndefinedAnyFunctionException);

  // TODO
  // REQUIRE_THROWS_WITH(f(), Catch::Matchers::Contains("undefined
  // AnyFunction"));

  SUBCASE("no return value") {
    auto value = 0;
    f = [&]() { value = 42; };

    REQUIRE(f.returnType() == getStaticTypeIndex<void>());
    REQUIRE(f.argumentCount() == 0);
    REQUIRE(f.argumentType(0) == getStaticTypeIndex<void>());
    REQUIRE(!f.isVariadic());

    REQUIRE_NOTHROW(f());
    REQUIRE(value == 42);
  }

  SUBCASE("return value") {
    f = []() -> int { return 42; };

    REQUIRE(f.returnType() == getStaticTypeIndex<int>());
    REQUIRE(f.argumentCount() == 0);
    REQUIRE(f.argumentType(0) == getStaticTypeIndex<void>());
    REQUIRE(!f.isVariadic());

    REQUIRE(f().get<int>() == 42);
  }

  REQUIRE(bool(f) == true);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  // TODO
  // REQUIRE_THROWS_WITH(f(1), Catch::Matchers::Contains("wrong number of
  // arguments"));
}

TEST_CASE("call with arguments") {
  AnyFunction f = [](int a, double b) { return a - b; };
  REQUIRE(f.returnType() == getStaticTypeIndex<double>());
  REQUIRE(f.argumentCount() == 2);
  REQUIRE(f.argumentType(0) == getStaticTypeIndex<int>());
  REQUIRE(f.argumentType(1) == getStaticTypeIndex<double>());
  REQUIRE(!f.isVariadic());

  REQUIRE(f(1, 2).type() == getStaticTypeIndex<double>());
  REQUIRE(f(1, 2).get<int>() == -1);
  REQUIRE(f(2, 1).get<int>() == 1);
  REQUIRE(f(1.5, 1).get<double>() == 0);
  REQUIRE(f(1, 1.5).get<double>() == -0.5);

  REQUIRE_THROWS_AS(f(), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1), AnyFunctionInvalidArgumentCountException);
  REQUIRE_THROWS_AS(f(1, 2, 3), AnyFunctionInvalidArgumentCountException);
}

TEST_CASE("call and modify reference arguments") {
  AnyFunction f = [](int &x) { x++; };
  int x = 41;
  f(x);
  REQUIRE(x == 42);
}

TEST_CASE("return any") {
  AnyFunction f = []() { return Any(42); };
  REQUIRE(f().get<int>() == 42);
}

TEST_CASE("pass any") {
  AnyFunction f = [](int x) { REQUIRE(x == 42); };
  Any x = 42;
  f(x);
}

TEST_CASE("take any") {
  SUBCASE("with value") {
    AnyFunction f = [](const Any &x) { REQUIRE(x.get<int>() == 42); };
    f(42);
  }
  SUBCASE("without value") {
    AnyFunction f = [](const Any &x) { REQUIRE(!x); };
    f(Any());
  }
}

TEST_CASE("call with any arguments") {
  SUBCASE("return value") {
    AnyFunction f = [](const AnyArguments &args) {
      double result = 0;
      for (auto &arg : args) {
        result += arg.get<double>();
      }
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

  SUBCASE("return void") {
    AnyFunction f = [](const AnyArguments &) {};
    REQUIRE(!f());
  }
}

TEST_CASE("implicit_string_conversion") {
  AnyFunction f = [](std::string v) -> std::string {
    return "Hello " + v + "!";
  };
  REQUIRE(f("AnyFunction").get<std::string>() == "Hello AnyFunction!");
}

TEST_CASE("any_function_with_std_function") {
  AnyFunction f = std::function<int()>([]() { return 42; });
  REQUIRE(f().get<int>() == 42);
}

TEST_CASE("any_with_any_function") {
  AnyFunction f = []() { return 42; };
  Any af = f;
  REQUIRE(af.get<const AnyFunction &>()().get<int>() == 42);
}

TEST_CASE("Automatic Casting") {
  struct A : public revisited::Visitable<A> {
    int value = 0;
  };
  struct B : public revisited::DerivedVisitable<B, A> {
    B() { value = 1; }
  };
  struct C : public revisited::DerivedVisitable<C, B> {
    C() { value = 2; }
  };

  SUBCASE("pass by reference") {
    AnyFunction f = [](A &x, A &y) {
      A a;
      a.value = x.value + y.value;
      return a;
    };
    REQUIRE(f(B(), C()).get<A &>().value == 3);
    REQUIRE(f(B(), C()).get<const A &>().value == 3);
    REQUIRE(f(std::make_shared<B>(), std::make_shared<C>()).get<A &>().value ==
            3);
    REQUIRE_THROWS(f(B(), C()).get<A>()); // currently a problem with visitable
                                          // types: cannot be captured by value
  }

  SUBCASE("pass by pointer") {
    AnyFunction f = [](std::shared_ptr<A> x, std::shared_ptr<A> y) {
      auto res = std::make_shared<A>();
      res->value = x->value + y->value;
      return res;
    };
    REQUIRE(f(B(), C()).get<A &>().value == 3);
    REQUIRE(f(B(), C()).get<const A &>().value == 3);
    REQUIRE(f(std::make_shared<B>(), std::make_shared<C>()).get<A &>().value ==
            3);
  }
}

TEST_CASE("non copy-constructable class") {
  struct A {
    int value;
    A(int v) : value(v) {}
    A(const A &) = delete;
    A(A &&) = default; // required to be std::function return value
  };
  revisited::AnyFunction f = []() { return A(3); };
  revisited::AnyFunction g = []() { return std::make_shared<A>(3); };
  REQUIRE(f().get<A &>().value == 3);
  REQUIRE(g().get<A &>().value == 3);
  REQUIRE(g().get<std::shared_ptr<A>>()->value == 3);
}

TEST_CASE("call with references") {
  AnyFunction f = [](const std::string &str) { REQUIRE(str == "420"); };
  std::string s = "420";
  SUBCASE("call with value") { f(s); }
  SUBCASE("call with reference") {
    std::string &ref = s;
    f(ref);
  }
  SUBCASE("call with const reference") {
    const std::string &ref = s;
    f(ref);
  }
}

TEST_CASE("passing shared pointers") {
  AnyFunction f = [](const std::shared_ptr<int> &p) {
    ++(*p);
    return p;
  };
  auto i = std::make_shared<int>(0);
  auto j = f(f(i)).get<std::shared_ptr<int>>();
  REQUIRE(*i == 2);
  REQUIRE(i == j);
}

TEST_CASE("return abstract pointer") {
  struct A {
    virtual int f() = 0;
    virtual ~A() {}
  };

  struct B : public A {
    int f() override { return 45; }
  };

  AnyFunction f = []() -> std::shared_ptr<A> { return std::make_shared<B>(); };

  AnyFunction g = [](std::shared_ptr<A> a) { return a->f(); };

  CHECK(g(f()).get<int>() == 45);
}