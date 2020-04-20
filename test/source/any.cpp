#include <doctest/doctest.h>
#include <revisited/any.h>

using namespace revisited;

TEST_CASE("AnyBasics"){
  Any v;

  SUBCASE("undefined"){
    CHECK(v.type() == getStaticTypeIndex<void>());
    CHECK(bool(v) == false);
    CHECK_THROWS_AS(v.get<int>(), UndefinedAnyException);
    // TODO: update for doctest
    // CHECK_THROWS_WITH(v.get<int>(), Catch::Matchers::Contains("undefined Any"));
  }

  SUBCASE("with value"){
    struct MyClass { 
      int value; 
      MyClass(int v):value(v){} 
      MyClass(const MyClass &) = default; 
    };

    CHECK(v.set<MyClass>(3).value == 3);

    SUBCASE("traits"){
      CHECK(v.type() == getStaticTypeIndex<MyClass>());
      CHECK(bool(v) == true);
    }

    SUBCASE("get"){
      CHECK(v.get<MyClass>().value == 3);
      CHECK(v.get<MyClass &>().value == 3);
      CHECK(v.get<const MyClass &>().value == 3);
    }

    SUBCASE("invalid get"){
      CHECK_THROWS_AS(v.get<int>(), InvalidVisitorException);
    }

    SUBCASE("try get"){
      CHECK(v.tryGet<MyClass>() == &v.get<MyClass &>());
      CHECK(v.tryGet<const MyClass>() == &v.get<MyClass &>());
      CHECK(v.tryGet<int>() == nullptr);
    }
  }

}

TEST_CASE("Reassign"){
  Any v = 1;
  CHECK_NOTHROW(v.get<int>());
  CHECK_THROWS(v.get<std::string>());

  SUBCASE("to value"){
    v = std::string("");
    CHECK_THROWS(v.get<int>());
    CHECK_NOTHROW(v.get<std::string>());
  }

  SUBCASE("to moved any"){
    Any o = 2;
    v = std::move(o);
    CHECK(v.get<int>() == 2);
  }

  SUBCASE("to temporary any"){
    v = Any(3);
    CHECK(v.get<int>() == 3);
  }

  SUBCASE("to temporary any reference"){
    v = AnyReference(4);
    CHECK(v.get<int>() == 4);
  }
}

TEST_CASE("Any string conversions"){
  Any v = "Hello any!";
  CHECK(v.get<std::string &>() == "Hello any!");
  CHECK(v.get<const std::string &>() == "Hello any!");
  CHECK(v.get<std::string>() == "Hello any!");
  CHECK(v.tryGet<std::string>()== &v.get<std::string &>());
  CHECK_THROWS_AS(v.get<int>(), InvalidVisitorException);
  CHECK(v.tryGet<int>() == nullptr);
}

TEST_CASE_TEMPLATE("Numerics", TestType, char, unsigned char, short int, unsigned short int, int, unsigned int, long int, unsigned long int, long long int, unsigned long long int, float, double, long double) {
  Any v;

  v.set<TestType>(42);

  CHECK(v.type() == getStaticTypeIndex<TestType>());
  
  CHECK(v.get<TestType &>() == 42);
  CHECK(v.get<const TestType &>() == 42);
  CHECK(v.get<char>() == 42);
  CHECK(v.get<int>() == 42);
  CHECK(v.get<unsigned>() == 42);
  CHECK(v.get<long>() == 42);
  CHECK(v.get<float>() == 42);
  CHECK(v.get<double>() == 42);
  CHECK(v.get<size_t>() == 42);
  CHECK_THROWS_AS(v.get<std::string>(), InvalidVisitorException);
}

TEST_CASE("floating point conversions"){
  Any v = 1.5;
  CHECK(v.get<double>() == 1.5);
  CHECK(v.get<int>() == 1);
  v = 3.141;
  CHECK(v.get<float>() == doctest::Approx(3.141));
}

TEST_CASE("String"){
  Any v;
  CHECK_THROWS_AS(v.get<std::string>(), UndefinedAnyException);

  SUBCASE("string"){
    v = std::string("Hello Any!");
    CHECK(v.get<std::string>() == "Hello Any!");
  }

  SUBCASE("string literal"){
    v = "Hello Any!";
    CHECK(v.get<std::string>() == "Hello Any!");
  }
  
  CHECK_THROWS_AS(v.get<int>(), InvalidVisitorException);
}

TEST_CASE("Inheritance"){
  struct A{ char a = 'A'; };
  struct B:public A{ char b = 'B'; };
  struct C:public B{ C(const C &) = delete; C() = default; char c = 'C'; };
  struct D{ char d = 'D'; };
  struct E: public C, public D{ char e; E(char v):e(v){ } };
  
  SUBCASE("Custom class"){
    Any v = A();
    CHECK(v.type() == getStaticTypeIndex<A>());
    CHECK(v.get<A>().a == 'A');
    CHECK(v.get<A &>().a == 'A');
    CHECK(v.get<const A &>().a == 'A');
    CHECK_THROWS_AS(v.get<B>(), InvalidVisitorException);
  }
  
  SUBCASE("Inheritance"){

    SUBCASE("set with bases"){
      Any v;
      v.setWithBases<E,D,C,B,A>('E');
      CHECK(v.get<A>().a == 'A');
      CHECK(v.get<B &>().b == 'B');
      CHECK(v.get<const C &>().c == 'C');
      CHECK(v.get<D>().d == 'D');
      CHECK(v.get<const E &>().e == 'E');
    }

    SUBCASE("create with bases"){
      auto v = Any::withBases<E,D,C,B,A>('E');
      CHECK(v.get<A>().a == 'A');
      CHECK(v.get<B &>().b == 'B');
      CHECK(v.get<const C &>().c == 'C');
      CHECK(v.get<D>().d == 'D');
      CHECK(v.get<const E &>().e == 'E');
    }

  }
}

TEST_CASE("Visitable inheritance"){
  struct A: Visitable<A> { char name = 'A'; };
  struct B: Visitable<B> { char name = 'B'; };
  struct C: public DerivedVisitable<C, A> { char name = 'C'; };
  struct D: public DerivedVisitable<D,VirtualVisitable<A, B>> { char name = 'D'; };
  struct E: public DerivedVisitable<E,VirtualVisitable<D, A>> { char name = 'E'; };
  Any v;
  SUBCASE("value"){
    v = makeAny<E>();
  } 
  SUBCASE("shared_ptr"){
    v = std::make_shared<E>();
  }
  CHECK(v.get<A &>().name == 'A');
  CHECK(v.get<const B &>().name == 'B');
  CHECK_THROWS_AS(v.get<C &>(), InvalidVisitorException);
  CHECK(v.get<D &>().name == 'D');
  CHECK(v.get<const E &>().name == 'E');
  CHECK(v.get<std::shared_ptr<A>>()->name == 'A');
  CHECK(v.get<std::shared_ptr<B>>()->name == 'B');
  CHECK_THROWS_AS(v.get<std::shared_ptr<C>>(), InvalidVisitorException);
  CHECK(v.get<std::shared_ptr<D>>()->name == 'D');
  CHECK(v.get<std::shared_ptr<E>>()->name == 'E');
}

TEST_CASE("capture reference"){
  int x = 1;
  Any y = std::reference_wrapper<int>(x);
  CHECK(&y.get<int &>() == &x);
  CHECK(&y.get<const int &>() == &x);
  CHECK(y.get<double>() == 1);
  y.get<int &>() = 2;
  CHECK(y.get<int>() == 2);
  CHECK(x == 2);
}

TEST_CASE("capture const reference"){
  int x = 1;
  Any y = std::reference_wrapper<const int>(x);
  CHECK_THROWS(y.get<int &>());
  CHECK(&y.get<const int &>() == &x);
  CHECK(y.get<double>() == 1);
}

TEST_CASE("AnyReference"){
  Any x = 1;
  AnyReference y;
  y = x;
  y.get<int &>() = 2;
  CHECK(x.get<int>() == 2);
  AnyReference z(Any(1));
  CHECK(z.get<int>() == 1);
  z = y;
  CHECK(z.get<int>() == 2);
  AnyReference a(z);
  CHECK(a.get<int>() == 2);
}

TEST_CASE("Accept visitors"){
  Any x = 1;
  
  SUBCASE("Visitor"){
    struct Visitor: revisited::Visitor<int &>{
      void visit(int &)override{ }
    } visitor;
    CHECK_NOTHROW(x.accept(visitor));
  }

  SUBCASE("ConstVisitor"){
    struct Visitor: revisited::Visitor<const int &>{
      void visit(const int &)override{ }
    } visitor;
    CHECK_NOTHROW(std::as_const(x).accept(visitor));
  }

  SUBCASE("RecursiveVisitor"){
    struct Visitor: revisited::RecursiveVisitor<int &>{
      bool visit(int &)override{ return true; }
    } visitor;
    CHECK(x.accept(visitor));
  }

  SUBCASE("ConstRecursiveVisitor"){
    struct Visitor: revisited::RecursiveVisitor<const int &>{
      bool visit(const int &)override{ return true; }
    } visitor;
    CHECK(std::as_const(x).accept(visitor));
  }

}

TEST_CASE("non cdefault-constructable class"){
  struct A {
    int value;
    A(int v):value(v){}
    A(const A &) = delete;
  };
  revisited::Any v;
  CHECK_NOTHROW(v.set<A>(3));
  CHECK(v.get<A&>().value == 3);
}

TEST_CASE("get shared pointers"){
  auto v = Any::create<int>(5);
  CHECK(v.getShared<int>());
  CHECK(*v.getShared<int>() == 5);
  CHECK(v.get<std::shared_ptr<int>>());
  CHECK(*v.get<std::shared_ptr<int>>() == 5);
  CHECK(v.get<double>() == 5);
  CHECK_THROWS(v.get<std::shared_ptr<double>>());
  CHECK(!v.getShared<double>());
}

TEST_CASE("set shared pointers"){
  SUBCASE("set to value"){
    auto s = std::make_shared<int>(3);
    Any v = s;
    CHECK(v.get<int>() == 3);
    CHECK(v.get<std::shared_ptr<int>>());
    CHECK(*v.get<std::shared_ptr<int>>() == 3);
    CHECK(*v.get<const std::shared_ptr<int> &>() == 3);
    CHECK(*v.get<std::shared_ptr<int> &>() == 3);
    CHECK(v.get<double>() == 3);
    CHECK(v.type() == revisited::getTypeIndex<int>());
  }

  SUBCASE("empty pointer"){
    Any v;
    CHECK_NOTHROW(v = std::shared_ptr<int>());
    CHECK(!bool(v));
    CHECK_THROWS(v.get<int>());
  }
  
}

TEST_CASE("set by reference"){
  Any v;
  SUBCASE("reference"){
    std::string s = "420";
    std::string &ref = s;
    v = ref;
    s = "";
  }
  SUBCASE("const reference"){
    std::string s = "420";
    const std::string &ref = s;
    v = ref;
    s = "";
  }
  CHECK(v.get<std::string>() == "420");
  CHECK(v.get<std::string &>() == "420");
  CHECK(v.get<const std::string &>() == "420");
}

namespace {
  struct A {
    int x = 1;
  };

  struct B: public A {
    B(){ x++; }
  };

  struct C: public B {
    C(){ x++; }
    C(const C &) = delete;
  };
}

LARS_ANY_DECLARE_BASES(B,A);
LARS_ANY_DECLARE_BASES(C,B);

TEST_CASE("base conversions"){
  revisited::Any v;
  v = A();
  CHECK(v.get<A &>().x == 1);
  CHECK_THROWS(v.get<B &>());
  CHECK_THROWS(v.get<C &>());

  v = revisited::makeAny<B>();
  CHECK(v.get<A &>().x == 2);
  CHECK(v.get<B &>().x == 2);
  CHECK_THROWS(v.get<C &>());

  v = revisited::makeAny<C>();
  CHECK(v.get<A &>().x == 3);
  CHECK(v.get<B &>().x == 3);
  CHECK(v.get<C &>().x == 3);
}
