
#include <catch2/catch.hpp>
#include <lars/visitor/inheritance_list.h>

#include <iostream>

namespace {
  using namespace lars::NEW;
 
  struct X: Visitable<X>{
  };

  struct A: Visitable<A> {
    char name = 'A';
  };
  
  struct B: Visitable<B> {
    char name = 'B';
  };
  
  struct C: public DerivedVisitable<C, A> {
    char name = 'C';
  };
  
  struct CX: public JointVisitable<C, X> {
  };
  
  struct BX: public DerivedVisitable<BX, JointVisitable<B, X>> {
  };
  
  struct ABCVisitor: public lars::NEW::Visitor<const A, const B, const C> {
    char result = 0;
    
    void visit(const A &v) override {
      REQUIRE(v.name == 'A');
      result = v.name;
    }
    
    void visit(const B &v) override {
      REQUIRE(v.name == 'B');
      result = v.name;
    }
    
    void visit(const C &v) override {
      REQUIRE(v.name == 'C');
      result = v.name;
    }

    char getTypeName(const VisitableBase &v) {
      result = 0;
      v.accept(*this);
      return result;
    }
    
  };

  struct ABXVisitor: public lars::NEW::RecursiveVisitor<A, B, X> {
    std::string result;
    
    bool visit(A &v) override {
      result += v.name;
      return true;
    }
    
    bool visit(B &v) override {
      result += v.name;
      return true;
    }
    
    bool visit(X &) override {
      result += 'X';
      return true;
    }
    
    auto getTypeName(VisitableBase &v) {
      result = "";
      v.accept(*this);
      return result;
    }
    
  };
}

TEST_CASE("New") {
  using namespace lars::NEW;

  std::shared_ptr<VisitableBase> a = std::make_shared<A>();
  std::shared_ptr<VisitableBase> b = std::make_shared<B>();
  std::shared_ptr<VisitableBase> c = std::make_shared<C>();
  std::shared_ptr<VisitableBase> x = std::make_shared<X>();
  std::shared_ptr<VisitableBase> cx = std::make_shared<CX>();
  std::shared_ptr<VisitableBase> bx = std::make_shared<BX>();

  ABCVisitor abcVisitor;
  ABXVisitor abxVisitor;

  REQUIRE(abcVisitor.asVisitorFor<const A>() == static_cast<SingleVisitor<const A>*>(&abcVisitor));
  REQUIRE(abcVisitor.asVisitorFor<const B>() == static_cast<SingleVisitor<const B>*>(&abcVisitor));
  REQUIRE(abcVisitor.asVisitorFor<const C>() == static_cast<SingleVisitor<const C>*>(&abcVisitor));
  REQUIRE(abcVisitor.asVisitorFor<X>() == nullptr);

  REQUIRE(abcVisitor.getTypeName(*a) == 'A');
  REQUIRE(abcVisitor.getTypeName(*b) == 'B');
  REQUIRE(abcVisitor.getTypeName(*c) == 'C');
  
  REQUIRE_THROWS_AS(abcVisitor.getTypeName(*x), IncompatibleVisitorException);
  REQUIRE_THROWS_WITH(abcVisitor.getTypeName(*x), Catch::Matchers::Contains("X") && Catch::Matchers::Contains("incompatible visitor"));
  REQUIRE(abxVisitor.getTypeName(*x) == "X");

  REQUIRE(abcVisitor.getTypeName(*cx) == 'C');
  REQUIRE(abxVisitor.getTypeName(*cx) == "AX");
  REQUIRE(abxVisitor.getTypeName(*bx) == "BX");
  
}
