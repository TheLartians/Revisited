#include <catch2/catch.hpp>
#include <lars/visitor.h>

TEST_CASE("Visitor") {
  using namespace lars;
  struct A:public Visitable<A>{ };
  struct B:public DVisitable<B,A>{ };
  struct C:public DVisitable<C,A>{ };
  struct D:public DVisitable<D,B,C>{ };
  struct E:public DVisitable<E,A>{ };
  struct F:public Visitable<F>{ };

  A a;
  B b;
  C c;
  D d;
  E e;
  F f;
  
  SECTION("Visitors"){
    struct TestVisitor: public Visitor<A,C,D> {
      char result = '0';
      void visit(A &)override{ result = 'a'; }
      void visit(C &)override{ result = 'c'; }
      void visit(D &)override{ result = 'd'; }
    };

    TestVisitor visitor;
    auto getType = [&](auto &a){
      a.accept(visitor);
      return visitor.result;
    };
    
    REQUIRE(getType(a) == 'a');
    REQUIRE(getType(b) == 'a');
    REQUIRE(getType(c) == 'c');
    REQUIRE(getType(d) == 'd');
    REQUIRE(getType(e) == 'a');
    REQUIRE_THROWS_AS(getType(f),IncompatibleVisitorException);
  }
  
  SECTION("Visitor Cast"){
    REQUIRE(visitor_cast<A>(&a) == &a);
    REQUIRE_FALSE(visitor_cast<B>(&a));
    REQUIRE_FALSE(visitor_cast<C>(&a));
    REQUIRE_FALSE(visitor_cast<D>(&a));
    REQUIRE_FALSE(visitor_cast<E>(&a));
    REQUIRE_FALSE(visitor_cast<F>(&a));

    REQUIRE(visitor_cast<A>(&b) == &b);
    REQUIRE(visitor_cast<B>(&b) == &b);
    REQUIRE_FALSE(visitor_cast<C>(&b));
    REQUIRE_FALSE(visitor_cast<D>(&b));
    REQUIRE_FALSE(visitor_cast<E>(&b));
    REQUIRE_FALSE(visitor_cast<F>(&b));

    REQUIRE(visitor_cast<A>(&c) == &c);
    REQUIRE_FALSE(visitor_cast<B>(&c));
    REQUIRE(visitor_cast<C>(&c) == &c);
    REQUIRE_FALSE(visitor_cast<D>(&c));
    REQUIRE_FALSE(visitor_cast<E>(&c));
    REQUIRE_FALSE(visitor_cast<F>(&c));

    REQUIRE(visitor_cast<A>(&d) == &d);
    REQUIRE(visitor_cast<B>(&d) == &d);
    REQUIRE(visitor_cast<C>(&d) == &d);
    REQUIRE(visitor_cast<D>(&d) == &d);
    REQUIRE_FALSE(visitor_cast<E>(&d));
    REQUIRE_FALSE(visitor_cast<F>(&d));

    REQUIRE(visitor_cast<A>(&e) == &e);
    REQUIRE_FALSE(visitor_cast<B>(&e));
    REQUIRE_FALSE(visitor_cast<C>(&e));
    REQUIRE_FALSE(visitor_cast<D>(&e));
    REQUIRE(visitor_cast<E>(&e) == &e);
    REQUIRE_FALSE(visitor_cast<F>(&e));

    REQUIRE_FALSE(visitor_cast<A>(&f));
    REQUIRE_FALSE(visitor_cast<B>(&f));
    REQUIRE_FALSE(visitor_cast<C>(&f));
    REQUIRE_FALSE(visitor_cast<D>(&f));
    REQUIRE_FALSE(visitor_cast<E>(&f));
    REQUIRE(visitor_cast<F>(&f) == &f);
  }
  
}
