#include <lars/visitor.h>
#include <memory>
#include <iostream>

struct Base: public virtual lars::VisitableBase{ };
struct A: public Base, public lars::Visitable<A> { };
struct B: public Base, public lars::Visitable<B> { };
struct C: public lars::DerivedVisitable<C, A> { };

struct ABVisitor: public lars::Visitor<A &,B &> {
  void visit(A &){ std::cout << "[AB ]: Visiting A" << std::endl; }
  void visit(B &){ std::cout << "[AB ]: Visiting B" << std::endl; }
};

struct ABCVisitor: public lars::Visitor<A &,B &, C &> {
  void visit(A &){ std::cout << "[ABC]: Visiting A" << std::endl; }
  void visit(B &){ std::cout << "[ABC]: Visiting B" << std::endl; }
  void visit(C &){ std::cout << "[ABC]: Visiting C" << std::endl; }
};

int main(){
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
