#include <revisited/any.h>
#include <iostream>

int main(){
  revisited::Any v = 42;
  
  std::cout << "value as int " << v.get<int>() << std::endl;
  std::cout << "value as double " << v.get<double>() << std::endl;

  v = "Hello Any!";
  std::cout << "stored a string: '" << v.get<std::string>() << "'" << std::endl;

  struct A{ char a = 'A'; };
  struct B:public A{ char b = 'B'; };
  struct C:public B{ char c = 'C'; };
  struct D{ char d = 'D'; };
  struct E: public C, public D { char e = 'E'; };
  
  v.setWithBases<E,D,C,B,A>();
  
  std::cout << "stored a value with inheritance:" << std::endl;
  std::cout << v.get<A>().a << std::endl;
  std::cout << v.get<B &>().b << std::endl;
  std::cout << v.get<const C &>().c << std::endl;
  std::cout << v.get<D>().d << std::endl;
  std::cout << v.get<E &>().e << std::endl;
  
  return 0;
}
