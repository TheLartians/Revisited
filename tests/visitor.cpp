
#include <catch2/catch.hpp>
#include <exception>

#include <lars/visitor.h>
#include <lars/visitor_pointer_cast.h>

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
  
  struct D: public DerivedVisitable<D,VirtualVisitable<A, B>> {
    char name = 'D';
  };
  
  struct E: public DerivedVisitable<E,VirtualVisitable<D, A, X>> {
    char name = 'E';
  };
  
  struct F: public DerivedVisitable<F,VirtualVisitable<B, E>> {
    char name = 'F';
  };
  
  struct BX: public DerivedVisitable<BX, JoinVisitable<B, X>> {
  };
  
  struct XB: public DerivedVisitable<XB, JoinVisitable<X, B>> {
  };

  struct CX: public JoinVisitable<C, X> {
  };
  
  struct XC: public VirtualVisitable<X, C> {
  };

  struct ABCVisitor: public lars::Visitor<const A &, const B &, const C &> {
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
  
  struct ABXVisitor: public lars::Visitor<A &, B &, X &> {
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
  
  struct ABCDRecursiveVisitor: public lars::RecursiveVisitor<A &, B &, C &, D &, E &, F &> {
    std::string result;
    bool non_recursive;

    bool visit(A &v) override {
      result += v.name;
      return non_recursive;
    }
    
    bool visit(B &v) override {
      result += v.name;
      return non_recursive;
    }
    
    bool visit(C &v) override {
      result += v.name;
      return non_recursive;
    }
    
    bool visit(D &v) override {
      result += v.name;
      return non_recursive;
    }
    
    bool visit(E &v) override {
      result += v.name;
      return non_recursive;
    }
    
    bool visit(F &v) override {
      result += v.name;
      return non_recursive;
    }
    
    struct Error: std::exception{};
    
    char getTypeName(VisitableBase &v) {
      result = "";
      non_recursive = true;
      v.accept(*this);
      if(result.size() != 1) throw Error();
      return result[0];
    }
    
    std::string getFullTypeName(VisitableBase &v) {
      result = "";
      non_recursive = false;
      v.accept(*this);
      return result;
    }
  };

}

TEST_CASE("visitor_basics", "[visitor]"){
  A a;
  B b;

  SECTION("by reference"){
    struct Visitor: public lars::Visitor <A &>{
      void visit(A &)override{}
    } visitor;
    REQUIRE_NOTHROW(a.accept(visitor));
    REQUIRE_THROWS_AS(std::as_const(a).accept(visitor), InvalidVisitorException);
    REQUIRE_THROWS_AS(b.accept(visitor), InvalidVisitorException);
  }

  SECTION("by const reference"){
    struct Visitor: public lars::Visitor <const A &>{
      void visit(const A &)override{}
    } visitor;
    REQUIRE_NOTHROW(a.accept(visitor));
    REQUIRE_NOTHROW(std::as_const(a).accept(visitor));
    REQUIRE_THROWS_AS(b.accept(visitor), InvalidVisitorException);
  }

  SECTION("recursive by reference"){
    struct Visitor: public lars::RecursiveVisitor <A &>{
      bool visit(A &)override{ return true; }
    } visitor;
    REQUIRE(a.accept(visitor));
    REQUIRE(!std::as_const(a).accept(visitor));
    REQUIRE(!b.accept(visitor));
  }

  SECTION("recursive by const reference"){
    struct Visitor: public lars::RecursiveVisitor <const A &>{
      bool visit(const A &)override{ return true; }
    } visitor;
    REQUIRE(a.accept(visitor));
    REQUIRE(std::as_const(a).accept(visitor));
    REQUIRE(!b.accept(visitor));
  }

}

TEST_CASE("visitor_inheritance", "[visitor]") {
  using namespace lars;
  
  std::shared_ptr<VisitableBase> a = std::make_shared<A>();
  std::shared_ptr<VisitableBase> b = std::make_shared<B>();
  std::shared_ptr<VisitableBase> c = std::make_shared<C>();
  std::shared_ptr<VisitableBase> d = std::make_shared<D>();
  std::shared_ptr<VisitableBase> e = std::make_shared<E>();
  std::shared_ptr<VisitableBase> f = std::make_shared<F>();
  std::shared_ptr<VisitableBase> x = std::make_shared<X>();
  std::shared_ptr<VisitableBase> bx = std::make_shared<BX>();
  std::shared_ptr<VisitableBase> xb = std::make_shared<XB>();
  std::shared_ptr<VisitableBase> cx = std::make_shared<CX>();
  std::shared_ptr<VisitableBase> xc = std::make_shared<XC>();
  
  SECTION("ABCVisitor"){
    ABCVisitor visitor;
    REQUIRE(visitor.asVisitorFor<const A &>() == static_cast<SingleVisitor<const A &>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<const B &>() == static_cast<SingleVisitor<const B &>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<const C &>() == static_cast<SingleVisitor<const C &>*>(&visitor));
    REQUIRE(visitor.asVisitorFor<X>() == nullptr);
    
    REQUIRE(visitor.getTypeName(*a) == 'A');
    REQUIRE(visitor.getTypeName(*b) == 'B');
    REQUIRE(visitor.getTypeName(*c) == 'C');
    REQUIRE_THROWS_AS(visitor.getTypeName(*x), InvalidVisitorException);
    REQUIRE_THROWS_WITH(visitor.getTypeName(*x), Catch::Matchers::Contains("X") && Catch::Matchers::Contains("invalid visitor"));
    REQUIRE(visitor.getTypeName(*d) == 'A');
    REQUIRE(visitor.getTypeName(*e) == 'A');
    REQUIRE(visitor.getTypeName(*f) == 'B');
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
    REQUIRE(visitor.getTypeName(*f) == 'B');
    REQUIRE(visitor.getTypeName(*x) == 'X');
    REQUIRE(visitor.getTypeName(*bx) == 'B');
    REQUIRE(visitor.getTypeName(*xb) == 'X');
    REQUIRE(visitor.getTypeName(*cx) == 'A');
    REQUIRE(visitor.getTypeName(*xc) == 'X');
  }
  
  SECTION("ABCDRecursiveVisitor"){
    ABCDRecursiveVisitor visitor;

    SECTION("getTypeName"){
      REQUIRE(visitor.getTypeName(*a) == 'A');
      REQUIRE(visitor.getTypeName(*b) == 'B');
      REQUIRE(visitor.getTypeName(*c) == 'C');
      REQUIRE(visitor.getTypeName(*d) == 'D');
      REQUIRE(visitor.getTypeName(*e) == 'E');
      REQUIRE(visitor.getTypeName(*f) == 'F');
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
      REQUIRE(visitor.getFullTypeName(*e) == "EDAB");
      REQUIRE(visitor.getFullTypeName(*f) == "FEDBA");
      REQUIRE(visitor.getFullTypeName(*x) == "");
      REQUIRE(visitor.getFullTypeName(*bx) == "B");
      REQUIRE(visitor.getFullTypeName(*xb) == "B");
      REQUIRE(visitor.getFullTypeName(*cx) == "CA");
      REQUIRE(visitor.getFullTypeName(*xc) == "CA");
    }
  }
  
}

template <class T, class V, class P> void testVisitorCastAs(V & v, P * p) {
  (void)p; // silence unused variable warning for g++
  if constexpr (std::is_base_of<T, P>::value) {
    REQUIRE(visitor_cast<T*>(&v) == p);
    REQUIRE(visitor_cast<T*>(nullptr) == nullptr);
    REQUIRE(&visitor_cast<T&>(v) == p);
    REQUIRE(visitor_cast<const T *>(&v) == p);
    REQUIRE(visitor_cast<const T *>(nullptr) == nullptr);
    REQUIRE(&visitor_cast<const T &>(v) == p);
  } else {
    REQUIRE(visitor_cast<T*>(&v) == nullptr);
    REQUIRE_THROWS(visitor_cast<T&>(v));
    REQUIRE(visitor_cast<const T *>(&v) == nullptr);
    REQUIRE_THROWS(visitor_cast<const T &>(v));
  }
}

template <class T, class P> void testVisitorCast(P & v) {
  return testVisitorCastAs<T, P, P>(v, &v);
}

TEMPLATE_TEST_CASE("Casting", "[visitor]", A, B, C, D, E ,F, BX, CX, XC){
  TestType t;
  testVisitorCast<A>(t);
  testVisitorCast<B>(t);
  testVisitorCast<C>(t);
  testVisitorCast<D>(t);
  testVisitorCast<E>(t);
  testVisitorCast<F>(t);
}

TEST_CASE("SharedVisitorCast", "[visitor]"){
  auto t = std::make_shared<A>();
  REQUIRE(visitor_pointer_cast<A>(t) == t);
  REQUIRE(visitor_pointer_cast<B>(t) == std::shared_ptr<B>());
}

TEST_CASE("Empty Visitable", "[visitor]"){
  EmptyVisitable v;
  REQUIRE_THROWS_AS(visitor_cast<int>(v), InvalidVisitorException);
  REQUIRE(visitor_cast<int *>(&v) == nullptr);
  REQUIRE_THROWS_AS(visitor_cast<int>(std::as_const(v)), InvalidVisitorException);
  REQUIRE(visitor_cast<const  int *>(&std::as_const(v)) == nullptr);
  REQUIRE(v.StaticTypeIndex() == getTypeIndex<void>());
  
  SECTION("Visitor"){
    Visitor<> visitor;
    REQUIRE_THROWS_AS(v.accept(visitor), InvalidVisitorException);
  }
  
  SECTION("RecursiveVisitor"){
    RecursiveVisitor<> visitor;
    REQUIRE(v.accept(visitor) == false);
  }

}

TEMPLATE_TEST_CASE("Data Visitable", "[visitor]", char, int, float, double, unsigned , size_t, long){
  using CastTypes = TypeList<TestType &>;
  using ConstCastTypes = TypeList<const TestType &, char, int, float, double, unsigned , size_t, long>;
  using DVisitable = DataVisitablePrototype<TestType, CastTypes, ConstCastTypes> ;
  
  DVisitable v(42);
  REQUIRE(v.StaticTypeIndex() == getTypeIndex<TestType>());

  SECTION("implicit value casting"){
    REQUIRE(visitor_cast<char>(v) == 42);
    REQUIRE(visitor_cast<int>(v) == 42);
    REQUIRE(visitor_cast<float>(v) == 42);
    REQUIRE(visitor_cast<double>(v) == 42);
    REQUIRE(visitor_cast<unsigned>(v) == 42);
    REQUIRE(visitor_cast<long>(v) == 42);
    REQUIRE_THROWS_AS(visitor_cast<bool>(v), InvalidVisitorException);
    REQUIRE_THROWS_AS(visitor_cast<std::string>(v), InvalidVisitorException);
  }
  
  SECTION("reference casting"){
    REQUIRE(visitor_cast<TestType &>(v) == 42);
    REQUIRE(visitor_cast<const TestType &>(v) == 42);
  }
  
  SECTION("accept visitor"){
    Visitor<> visitor;
    REQUIRE_THROWS_AS(v.accept(visitor), InvalidVisitorException);
    REQUIRE_THROWS_AS(std::as_const(v).accept(visitor), InvalidVisitorException);
  }
}
