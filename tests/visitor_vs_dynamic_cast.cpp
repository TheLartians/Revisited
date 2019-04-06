#include <iostream>

#include <lars/log.h>
#define LOG LARS_LOG

#include <lars/visitor.h>
#include <lars/timeit.h>

namespace visitable{

  using namespace lars;

  struct A:public Visitable<A>{ };
  struct B:public DVisitable<B,A>{ };
  struct C:public DVisitable<C,A>{ };
  struct D:public DVisitable<D,B,C>{ };
  struct E:public DVisitable<E,A>{ };

  volatile A * va;

  A * obfuscate(A &a){
    va = &a;
    return (A*) va;
  }

  D * __attribute__ ((noinline)) perform_cast(A &a){
    return visitor_cast<D>(obfuscate(a));
  }

}

namespace dynamic{
  
  struct A{ virtual ~A(){} };
  struct B:public virtual A{ };
  struct C:public virtual A{ };
  struct D:public virtual B,public virtual C{ };
  struct E:public virtual A{ };
  
  volatile A * va;

  A * obfuscate(A &a){
    va = &a;
    return (A*) va;
  }
  
  D * __attribute__ ((noinline)) perform_cast(A &a){
    return dynamic_cast<D*>(obfuscate(a));
  }
  
}



int main(){
  
  visitable::D vd;
  dynamic::D dd;
  
  lars::time_it([&](){ visitable::perform_cast(vd); });
  std::cout << "visitor cast: " << lars::time_it([&](){ visitable::perform_cast(vd); }) << std::endl;
  
  lars::time_it([&](){ dynamic::perform_cast(dd); });
  std::cout << "dynamic cast: " << lars::time_it([&](){ dynamic::perform_cast(dd); }) << std::endl;

  return 0;
}
