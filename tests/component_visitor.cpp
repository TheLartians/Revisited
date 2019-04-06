#include <catch2/catch.hpp>
#include <lars/component_visitor.h>

TEST_CASE("Component visitor") {
  using namespace lars;
  
  struct A:public Visitable<A>{ };
  struct B:public DVisitable<B,A>{ };
  struct C:public DVisitable<C,A>{ };
  struct D:public DVisitable<D,B,C>{ };

  D d;

  ComponentVisitor visitor;

  char current = 0;
  visitor.add_visitor<A>([&](A &){ current = 'a'; });
  d.accept(visitor);
  REQUIRE(current == 'a');

  visitor.add_visitor<B>([&](B &){ current = 'b';; });
  d.accept(visitor);
  REQUIRE(current == 'b');

  visitor.add_visitor<D>([&](D &){ current = 'd'; });
  d.accept(visitor);
  REQUIRE(current == 'd');

  visitor.remove_visitor<D>();
  d.accept(visitor);
  REQUIRE(current == 'b');

  ComponentVisitable v;
  v.create_component<A>();
  v.create_component<B>();
  
  v.accept(visitor);
  REQUIRE(current == 'b');

  v.remove_component<B>();
  v.accept(visitor);
  REQUIRE(current == 'a');
}
