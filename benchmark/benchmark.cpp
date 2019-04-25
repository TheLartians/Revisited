#include <lars/visitor.h>
#include <lars/timeit.h>

#include <memory>
#include <benchmark/benchmark.h>

#include <iostream>

namespace classic {
  struct B;
  struct E;
  
  struct BEVisitor{
    virtual void visit(B &) = 0;
    virtual void visit(E &) = 0;
  };
  
  struct A{
    char a = 'A';
    virtual void accept(BEVisitor &v) = 0;
    virtual ~A(){}
  };
  
  struct B:public virtual A{
    char b = 'B';
    void accept(BEVisitor &v)override{ v.visit(*this); }
  };
  
  struct C:public virtual A{ char c = 'C'; };
  struct D:public B,public C{ D(){ a = b = c = 'D'; }; };
  
  struct E:public A{
    char e = 'E';
    void accept(BEVisitor &v)override{ v.visit(*this); }
  };
  
  struct BOrEVisitor: public BEVisitor{
    char result;
    void visit(B &b)override{ result = b.b; }
    void visit(E &e)override{ result = e.e; }
  };
  
  char __attribute__ ((noinline)) getValue (A & a){
    BOrEVisitor visitor;
    a.accept(visitor);
    return visitor.result;
  }
}

namespace dynamic {
  char __attribute__ ((noinline)) getValue (classic::A & a){
    if (auto b = dynamic_cast<classic::B*>(&a)) {
      return b->b;
    } else {
      return dynamic_cast<classic::E&>(a).e;
    }
  }
}

namespace visitor {
  using namespace lars;
  
  struct A:public Visitable<A>{ char a = 'a'; };
  struct B:public DerivedVisitable<B,VirtualVisitable<A>>{ char b = 'B'; };
  struct C:public DerivedVisitable<C,VirtualVisitable<A>>{ char c = 'C'; };
  struct D:public DerivedVisitable<D,JoinVisitable<B,C>>{ D(){ a = b = c = 'D'; }; };
  struct E:public DerivedVisitable<E,A>{ char e = 'E'; };
  
  struct BOrEVisitor:public Visitor<B &,E &>{
    char result;
    void visit(B &b)override{ result = b.b; }
    void visit(E &e)override{ result = e.e; }
  };
  
  char __attribute__ ((noinline)) getValue (A & a){
    BOrEVisitor visitor;
    a.accept(visitor);
    return visitor.result;
  }
}

bool Assert(bool v){
  if (!v) { throw std::runtime_error("assertion failed"); }
  return v;
}

static void ClassicVisitor(benchmark::State& state) {
  using namespace classic;
  std::shared_ptr<A> b = std::make_shared<B>();
  std::shared_ptr<A> d = std::make_shared<D>();
  std::shared_ptr<A> e = std::make_shared<E>();

  for (auto _ : state) {
    Assert(getValue(*b) == 'B');
    Assert(getValue(*d) == 'D');
    Assert(getValue(*e) == 'E');
  }
}

static void LarsVisitor(benchmark::State& state) {
  using namespace visitor;
  std::shared_ptr<A> b = std::make_shared<B>();
  std::shared_ptr<A> d = std::make_shared<D>();
  std::shared_ptr<A> e = std::make_shared<E>();

  for (auto _ : state) {
    Assert(getValue(*b) == 'B');
    Assert(getValue(*d) == 'D');
    Assert(getValue(*e) == 'E');
  }
}

static void DynamicVisitor(benchmark::State& state) {
  using namespace classic;
  std::shared_ptr<A> b = std::make_shared<B>();
  std::shared_ptr<A> d = std::make_shared<D>();
  std::shared_ptr<A> e = std::make_shared<E>();

  for (auto _ : state) {
    Assert(dynamic::getValue(*b) == 'B');
    Assert(dynamic::getValue(*d) == 'D');
    Assert(dynamic::getValue(*e) == 'E');
  }
}

static void VisitorCast(benchmark::State& state) {
  using namespace visitor;
  std::shared_ptr<A> b = std::make_shared<B>();
  std::shared_ptr<A> e = std::make_shared<E>();

  for (auto _ : state) {
    benchmark::DoNotOptimize(Assert(lars::visitor_cast<B *>(b.get())));
    benchmark::DoNotOptimize(Assert(!lars::visitor_cast<B *>(e.get())));
    benchmark::DoNotOptimize(Assert(lars::visitor_cast<E *>(e.get())));
    benchmark::DoNotOptimize(Assert(!lars::visitor_cast<B *>(e.get())));
  }
}

static void DynamicCast(benchmark::State& state) {
  using namespace classic;
  std::shared_ptr<A> b = std::make_shared<B>();
  std::shared_ptr<A> e = std::make_shared<E>();

  for (auto _ : state) {
    benchmark::DoNotOptimize(Assert(dynamic_cast<B *>(b.get())));
    benchmark::DoNotOptimize(Assert(!dynamic_cast<B *>(e.get())));
    benchmark::DoNotOptimize(Assert(dynamic_cast<E *>(e.get())));
    benchmark::DoNotOptimize(Assert(!dynamic_cast<B *>(e.get())));
  }
}


BENCHMARK(ClassicVisitor);
BENCHMARK(LarsVisitor);
BENCHMARK(DynamicVisitor);

BENCHMARK(VisitorCast);
BENCHMARK(DynamicCast);

BENCHMARK_MAIN();
