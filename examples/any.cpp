#include <lars/any.h>
#include <iostream>

int main(){
  lars::Any v = 42;
  
  std::cout << "value as int " << v.get<int>() << std::endl;
  std::cout << "value as double " << v.get<double>() << std::endl;

  v = "Hello Any!";
  std::cout << "stored a string: '" << v.get<std::string>() << "'" << std::endl;

  struct A{ char name = 'A'; };
  struct B:public A{ char name = 'B'; };
  struct C:public B{ char name = 'C'; };
  struct D{ char name = 'D'; };
  struct E: public C, public D { char name = 'E'; };
  
  v.setWithBases<E,D,C,B,A>();
  
  std::cout << "stored a value with inheritance:" << std::endl;
  std::cout << v.get<A>().name << std::endl;
  std::cout << v.get<B &>().name << std::endl;
  std::cout << v.get<const C &>().name << std::endl;
  std::cout << v.get<D>().name << std::endl;
  std::cout << v.get<E &>().name << std::endl;
  
  return 0;
}
