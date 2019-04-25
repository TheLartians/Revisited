#include <lars/visitor.h>
#include <lars/timeit.h>

#include <benchmark/benchmark.h>

#include <iostream>

template <class T> void doNotOptimizeAway(T && v) {
  asm volatile("" :: "g" (v));
}

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
    doNotOptimizeAway(a);
    BOrEVisitor visitor;
    a.accept(visitor);
    return visitor.result;
  }
}

namespace dynamic {
  char __attribute__ ((noinline)) getValue (classic::A & a){
    doNotOptimizeAway(a);
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
    doNotOptimizeAway(a);
    BOrEVisitor visitor;
    a.accept(visitor);
    return visitor.result;
  }
}

void Assert(bool v){
  if (!v) {
    throw std::runtime_error("assertion failed");
  }
}


static void BM_SomeFunction(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    SomeFunction();
  }
}
// Register the function as a benchmark
BENCHMARK(BM_SomeFunction);
// Run the benchmark
BENCHMARK_MAIN();

/*
int main(){
  do {
    using namespace classic;
    std::shared_ptr<A> b = std::make_shared<B>();
    std::shared_ptr<A> d = std::make_shared<D>();
    std::shared_ptr<A> e = std::make_shared<E>();
    doNotOptimizeAway(b);
    doNotOptimizeAway(d);
    doNotOptimizeAway(e);
    
    auto result = lars::time_it([&](){
      Assert(getValue(*b) == 'B');
      Assert(getValue(*d) == 'D');
      Assert(getValue(*e) == 'E');
    });
    
    std::cout << "classical visitor: " << result << std::endl;
    
  } while (false);
  
  return 0;
}
*/