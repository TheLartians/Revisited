#include <catch2/catch.hpp>
#include <lars/component_visitor.h>

TEST_CASE("Component visitor") {
  using namespace lars;
  
  struct A:public Visitable<A>{ };
  struct B:public DVisitable<B,A>{ };
  struct C:public DVisitable<C,A>{ };
  struct D:public DVisitable<D,B,C>{ };
  struct E:public DVisitable<E,A>{ };
  
}

/*
int main(){
  D d;

  ComponentVisitor visitor;
  visitor.add_visitor<A>([](A &a){ cout << "visit a: " << &a << endl; });
  d.accept(visitor);
  
  visitor.add_visitor<B>([](B &a){ cout << "visit b: " << &a << endl; });
  d.accept(visitor);
  
  visitor.add_visitor<D>([](D &a){ cout << "visit d: " << &a << endl; });
  d.accept(visitor);

  visitor.remove_visitor<D>();
  d.accept(visitor);
  
  ComponentVisitable v;
  v.create_component<A>();
  v.create_component<B>();
  
  v.accept(visitor);
  v.remove_component<B>();
  v.accept(visitor);
}
*/
