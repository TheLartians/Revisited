#include <revisited/any_function.h>
#include <iostream>

int main() {
  revisited::AnyFunction f;

  // creating and calling any function
  f = [](int x, double y){ return x + y; };
  std::cout << "f(2,3) = " << f(2,3).get<int>() << std::endl;
  
  // passing parameter by reference
  f = [](int &x){ x++; };
  int x = 0;
  f(x);
  std::cout << "x = " << x << std::endl;

  // variadiac arguments
  f = [](const revisited::AnyArguments &args){
    double result = 0;
    for(auto &arg: args) { result += arg.get<double>(); }
    return result;
  };
  std::cout << "f(1,2,3,4,5) = " << f(1,2,3,4,5).get<int>() << std::endl;
  
  return 1;
}
