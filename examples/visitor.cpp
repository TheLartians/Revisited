#include <iostream>
#include <memory>
#include <revisited/visitor.h>

struct Base : public virtual revisited::VisitableBase {};
struct A : public Base, public revisited::Visitable<A> {};
struct B : public Base, public revisited::Visitable<B> {};
struct C : public revisited::DerivedVisitable<C, A> {};

struct ABVisitor : public revisited::Visitor<const A &, const B &> {
  void visit(const A &) { std::cout << "[AB ]: Visiting A" << std::endl; }
  void visit(const B &) { std::cout << "[AB ]: Visiting B" << std::endl; }
};

struct ABCVisitor : public revisited::Visitor<A &, B &, C &> {
  void visit(A &) { std::cout << "[ABC]: Visiting A" << std::endl; }
  void visit(B &) { std::cout << "[ABC]: Visiting B" << std::endl; }
  void visit(C &) { std::cout << "[ABC]: Visiting C" << std::endl; }
};

int main() {
  std::shared_ptr<Base> a = std::make_shared<A>();
  std::shared_ptr<Base> b = std::make_shared<B>();
  std::shared_ptr<Base> c = std::make_shared<C>();

  ABVisitor abVisitor;
  a->accept(abVisitor);
  b->accept(abVisitor);
  c->accept(abVisitor);

  ABCVisitor abcVisitor;
  a->accept(abcVisitor);
  b->accept(abcVisitor);
  c->accept(abcVisitor);

  return 0;
}
