
#include <catch2/catch.hpp>
#include <exception>

#include <lars/visitor.h>

namespace {
  using namespace lars;
  
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
  
  struct D: public DerivedVisitable<D,VirtualJoinVisitable<A, B>> {
    char name = 'D';
  };
  
  struct E: public DerivedVisitable<E,VirtualJoinVisitable<A, B, D, X>> {
    char name = 'D';
  };
  
  struct BX: public DerivedVisitable<BX, JoinVisitable<B, X>> {
  };
  
  struct XB: public DerivedVisitable<XB, JoinVisitable<X, B>> {
  };

  struct CX: public JoinVisitable<C, X> {
  };
  
  struct XC: public JoinVisitable<X, C> {
  };

  struct ABCVisitor: public lars::Visitor<const A, const B, const C> {
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
  
  struct ABXVisitor: public lars::Visitor<A, B, X> {
    char result = 0;

    void visit(A &v) override {
      result += v.name;
    }
    
    void visit(B &v) override {
      result += v.name;
    }
    
    void visit(X &) override {
      result += 'X';
    }
    
    char getTypeName(VisitableBase &v) {
      result = 0;
      v.accept(*this);
      return result;
    }
  };
  
  struct ABCDRecursiveVisitor: public lars::RecursiveVisitor<const A, const B, const C, const D> {
    std::string result;
    bool recursive;

    bool visit(const A &v) override {
      result += v.name;
      return recursive;
    }
    
    bool visit(const B &v) override {
      result += v.name;
      return recursive;
    }
    
    bool visit(const C &v) override {
      result += v.name;
      return recursive;
    }
    
    bool visit(const D &v) override {
      result += v.name;
      return recursive;
    }
    
    struct Error: std::exception{};
    
    char getTypeName(const VisitableBase &v) {
      result = "";
      recursive = false;
      v.accept(*this);
      if(result.size() != 1) throw Error();
      return result[0];
    }
    
    std::string getFullTypeName(const VisitableBase &v) {
      result = "";
      recursive = true;
      v.accept(*this);
      return result;
    }
  };

}

TEST_CASE("Visitor") {
  using namespace lars;
  
  std::shared_ptr<VisitableBase> a = std::make_shared<A>();
  std::shared_ptr<VisitableBase> b = std::make_shared<B>();
  std::shared_ptr<VisitableBase> c = std::make_shared<C>();
  std::shared_ptr<VisitableBase> d = std::make_shared<D>();
  std::shared_ptr<VisitableBase> e = std::make_shared<E>();
  std::shared_ptr<VisitableBase> x = std::make_shared<X>();
  std::shared_ptr<VisitableBase> bx = std::make_shared<BX>();
  std::shared_ptr<VisitableBase> xb = std::make_shared<XB>();
  std::shared_ptr<VisitableBase> cx = std::make_shared<CX>();
  std::shared_ptr<VisitableBase> xc = std::make_shared<XC>();

  SECTION("ABCVisitor"){
    ABCVisitor visitor;
    REQUIRE(visitor.asVisitorFor<const A>() == static_cast<SingleVisitor<const A>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<const B>() == static_cast<SingleVisitor<const B>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<const C>() == static_cast<SingleVisitor<const C>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<X>() == nullptr);
    
    REQUIRE(visitor.getTypeName(*a) == 'A');
    REQUIRE(visitor.getTypeName(*b) == 'B');
    REQUIRE(visitor.getTypeName(*c) == 'C');
    REQUIRE_THROWS_AS(visitor.getTypeName(*x), IncompatibleVisitorException);
    REQUIRE_THROWS_WITH(visitor.getTypeName(*x), Catch::Matchers::Contains("X") && Catch::Matchers::Contains("incompatible visitor"));
    REQUIRE(visitor.getTypeName(*d) == 'A');
    REQUIRE(visitor.getTypeName(*e) == 'A');
    REQUIRE(visitor.getTypeName(*bx) == 'B');
    REQUIRE(visitor.getTypeName(*xb) == 'B');
    REQUIRE(visitor.getTypeName(*cx) == 'C');
    REQUIRE(visitor.getTypeName(*xc) == 'C');
 }
  
  SECTION("ABXVisitor"){
    ABXVisitor visitor;
    
    REQUIRE(visitor.getTypeName(*a) == 'A');
    REQUIRE(visitor.getTypeName(*b) == 'B');
    REQUIRE(visitor.getTypeName(*c) == 'A');
    REQUIRE(visitor.getTypeName(*d) == 'A');
    REQUIRE(visitor.getTypeName(*e) == 'A');
    REQUIRE(visitor.getTypeName(*x) == 'X');
    REQUIRE(visitor.getTypeName(*bx) == 'B');
    REQUIRE(visitor.getTypeName(*xb) == 'X');
    REQUIRE(visitor.getTypeName(*cx) == 'A');
    REQUIRE(visitor.getTypeName(*xc) == 'X');
  }
  
  SECTION("ABCXRecursiveVisitor"){
    ABCDRecursiveVisitor visitor;

    SECTION("getTypeName"){
      REQUIRE(visitor.getTypeName(*a) == 'A');
      REQUIRE(visitor.getTypeName(*b) == 'B');
      REQUIRE(visitor.getTypeName(*c) == 'C');
      REQUIRE(visitor.getTypeName(*d) == 'D');
      // REQUIRE(visitor.getTypeName(*e) == 'D'); TODO
      REQUIRE_THROWS_AS(visitor.getTypeName(*x), ABCDRecursiveVisitor::Error);
      REQUIRE(visitor.getTypeName(*bx) == 'B');
      REQUIRE(visitor.getTypeName(*xb) == 'B');
      REQUIRE(visitor.getTypeName(*cx) == 'C');
      REQUIRE(visitor.getTypeName(*xc) == 'C');
    }
    
    SECTION("getFullTypeName"){
      REQUIRE(visitor.getFullTypeName(*a) == "A");
      REQUIRE(visitor.getFullTypeName(*b) == "B");
      REQUIRE(visitor.getFullTypeName(*c) == "CA");
      REQUIRE(visitor.getFullTypeName(*d) == "DAB");
      // REQUIRE(visitor.getFullTypeName(*e) == "DBA"); TODO
      REQUIRE(visitor.getFullTypeName(*x) == "");
      REQUIRE(visitor.getFullTypeName(*bx) == "B");
      REQUIRE(visitor.getFullTypeName(*xb) == "B");
      REQUIRE(visitor.getFullTypeName(*cx) == "CA");
      REQUIRE(visitor.getFullTypeName(*xc) == "CA");
    }
  }
  
}
