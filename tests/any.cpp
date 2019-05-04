#include <catch2/catch.hpp>
#include <lars/to_string.h>

#include <lars/any.h>

using namespace lars;

TEST_CASE("AnyBasics", "[any]"){
  Any v;

  SECTION("undefined"){
    REQUIRE(v.type() == getStaticTypeIndex<void>());
    REQUIRE(bool(v) == false);
    REQUIRE_THROWS_AS(v.get<int>(), UndefinedAnyException);
    REQUIRE_THROWS_WITH(v.get<int>(), Catch::Matchers::Contains("undefined Any"));
  }

  SECTION("with value"){
    struct MyClass { 
      int value; 
      MyClass(int v):value(v){} 
      MyClass(const MyClass &) = default; 
    };

    REQUIRE(v.set<MyClass>(3).value == 3);

    SECTION("traits"){
      REQUIRE(v.type() == getStaticTypeIndex<MyClass>());
      REQUIRE_THAT(stream_to_string(v), Catch::Matchers::Contains("Any") && Catch::Matchers::Contains("MyClass"));
      REQUIRE(bool(v) == true);
    }

    SECTION("get"){
      REQUIRE(v.get<MyClass>().value == 3);
      REQUIRE(v.get<MyClass &>().value == 3);
      REQUIRE(v.get<const MyClass &>().value == 3);
    }

    SECTION("invalid get"){
      REQUIRE_THROWS_AS(v.get<int>(), InvalidVisitorException);
    }

    SECTION("try get"){
      REQUIRE(v.tryGet<MyClass>() == &v.get<MyClass &>());
      REQUIRE(v.tryGet<const MyClass>() == &v.get<MyClass &>());
      REQUIRE(v.tryGet<int>() == nullptr);
    }
  }

}

TEST_CASE("Reassign", "[any]"){
  Any v = 1;
  REQUIRE_NOTHROW(v.get<int>());
  REQUIRE_THROWS(v.get<std::string>());

  SECTION("to value"){
    v = std::string("");
    REQUIRE_THROWS(v.get<int>());
    REQUIRE_NOTHROW(v.get<std::string>());
  }

  SECTION("to moved any"){
    Any o = 2;
    v = std::move(o);
    REQUIRE(v.get<int>() == 2);
  }

  SECTION("to temporary any"){
    v = Any(3);
    REQUIRE(v.get<int>() == 3);
  }

  SECTION("to temporary any reference"){
    v = AnyReference(4);
    REQUIRE(v.get<int>() == 4);
  }
}

TEST_CASE("Any string conversions", "[any]"){
  Any v = "Hello any!";
  REQUIRE(v.get<std::string &>() == "Hello any!");
  REQUIRE(v.get<const std::string &>() == "Hello any!");
  REQUIRE(v.get<std::string>() == "Hello any!");
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
  struct A{ char a = 'A'; };
  struct B:public A{ char b = 'B'; };
  struct C:public B{ C(const C &) = delete; C() = default; char c = 'C'; };
  struct D{ char d = 'D'; };
  struct E: public C, public D{ char e; E(char v):e(v){ } };
  
  SECTION("Custom class"){
    Any v = A();
    REQUIRE(v.type() == getStaticTypeIndex<A>());
    REQUIRE(v.get<A>().a == 'A');
    REQUIRE(v.get<A &>().a == 'A');
    REQUIRE(v.get<const A &>().a == 'A');
    REQUIRE_THROWS_AS(v.get<B>(), InvalidVisitorException);
  }
  
  SECTION("Inheritance"){

    SECTION("set with bases"){
      Any v;
      v.setWithBases<E,D,C,B,A>('E');
      REQUIRE(v.get<A>().a == 'A');
      REQUIRE(v.get<B &>().b == 'B');
      REQUIRE(v.get<const C &>().c == 'C');
      REQUIRE(v.get<D>().d == 'D');
      REQUIRE(v.get<const E &>().e == 'E');
    }

    SECTION("create with bases"){
      auto v = Any::withBases<E,D,C,B,A>('E');
      REQUIRE(v.get<A>().a == 'A');
      REQUIRE(v.get<B &>().b == 'B');
      REQUIRE(v.get<const C &>().c == 'C');
      REQUIRE(v.get<D>().d == 'D');
      REQUIRE(v.get<const E &>().e == 'E');
    }

  }
}

TEST_CASE("Visitable inheritance","[any]"){
  struct A: Visitable<A> { char name = 'A'; };
  struct B: Visitable<B> { char name = 'B'; };
  struct C: public DerivedVisitable<C, A> { char name = 'C'; };
  struct D: public DerivedVisitable<D,VirtualVisitable<A, B>> { char name = 'D'; };
  struct E: public DerivedVisitable<E,VirtualVisitable<D, A>> { char name = 'E'; };
  auto v = makeAny<E>();
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
  REQUIRE(y.get<double>() == 1);
  y.get<int &>() = 2;
  REQUIRE(y.get<int>() == 2);
  REQUIRE(x == 2);
}

TEST_CASE("AnyReference","[any]"){
  Any x = 1;
  AnyReference y;
  y = x;
  y.get<int &>() = 2;
  REQUIRE(x.get<int>() == 2);
  AnyReference z(Any(1));
  REQUIRE(z.get<int>() == 1);
  z = y;
  REQUIRE(z.get<int>() == 2);
  AnyReference a(z);
  REQUIRE(a.get<int>() == 2);
}

TEST_CASE("Accept visitors","[any]"){
  Any x = 1;
  
  SECTION("Visitor"){
    struct Visitor: lars::Visitor<int &>{
      void visit(int &)override{ }
    } visitor;
    REQUIRE_NOTHROW(x.accept(visitor));
  }

  SECTION("ConstVisitor"){
    struct Visitor: lars::Visitor<const int &>{
      void visit(const int &)override{ }
    } visitor;
    REQUIRE_NOTHROW(std::as_const(x).accept(visitor));
  }

  SECTION("RecursiveVisitor"){
    struct Visitor: lars::RecursiveVisitor<int &>{
      bool visit(int &)override{ return true; }
    } visitor;
    REQUIRE(x.accept(visitor));
  }

  SECTION("ConstRecursiveVisitor"){
    struct Visitor: lars::RecursiveVisitor<const int &>{
      bool visit(const int &)override{ return true; }
    } visitor;
    REQUIRE(std::as_const(x).accept(visitor));
  }

}

TEST_CASE("non cdefault-constructable class", "[any]"){
  struct A {
    int value;
    A(int v):value(v){}
    A(const A &) = delete;
  };
  lars::Any v;
  REQUIRE_NOTHROW(v.set<A>(3));
  REQUIRE_NOTHROW(v.get<A&>().value == 3);
}

TEST_CASE("get shared pointers", "[any]"){
  auto v = Any::create<int>(5);
  REQUIRE(v.getShared<int>());
  REQUIRE(*v.getShared<int>() == 5);
  REQUIRE(v.get<std::shared_ptr<int>>());
  REQUIRE(*v.get<std::shared_ptr<int>>() == 5);
  REQUIRE(v.get<double>() == 5);
}

TEST_CASE("set shared pointers", "[any]"){
  auto s = std::make_shared<int>(3);
  Any v = s;
  REQUIRE(v.get<int>() == 3);
  REQUIRE(v.get<std::shared_ptr<int>>());
  REQUIRE(*v.get<std::shared_ptr<int>>() == 3);
  REQUIRE(v.get<double>() == 3);
}
