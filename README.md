[![Actions Status](https://github.com/TheLartians/Revisited/workflows/MacOS/badge.svg)](https://github.com/TheLartians/Revisited/actions)
[![Actions Status](https://github.com/TheLartians/Revisited/workflows/Windows/badge.svg)](https://github.com/TheLartians/Revisited/actions)
[![Actions Status](https://github.com/TheLartians/Revisited/workflows/Ubuntu/badge.svg)](https://github.com/TheLartians/Revisited/actions)
[![Actions Status](https://github.com/TheLartians/Revisited/workflows/Style/badge.svg)](https://github.com/TheLartians/Revisited/actions)
[![Actions Status](https://github.com/TheLartians/Revisited/workflows/Install/badge.svg)](https://github.com/TheLartians/Revisited/actions)
[![codecov](https://codecov.io/gh/TheLartians/Revisited/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/Revisited)

# Revisited

A C++17 acyclic visitor template and inheritance-aware any and any-function class. Using revisited::Visitor greatly reduces the boilerplate code required for implementing the [visitor pattern](https://en.wikipedia.org/wiki/Visitor_pattern) in C++. It uses only [compile time type information](https://github.com/Manu343726/ctti) and has better performance than solutions relying on run time type information such as `dynamic_cast`.

## Examples

See the [examples directory](https://github.com/TheLartians/Visitor/tree/master/examples) for full examples.

### Revisited Examples

#### Simple Visitor

```cpp
#include <memory>
#include <iostream>
#include <revisited/visitor.h>

struct Base: public virtual revisited::VisitableBase { };
struct A: public Base, public revisited::Visitable<A> { };
struct B: public Base, public revisited::Visitable<B> { };

struct Visitor: public revisited::Visitor<A &,B &> {
  void visit(A &){ std::cout << "Visiting A" << std::endl; }
  void visit(B &){ std::cout << "Visiting B" << std::endl; }
};

int main() {
  std::shared_ptr<Base> a = std::make_shared<A>();
  std::shared_ptr<Base> b = std::make_shared<B>();
  
  Visitor visitor;
  a->accept(visitor); // -> Visiting A
  b->accept(visitor); // -> Visiting B
}
```

#### Derived Classes

revisited::Visitor also understands derived classes and classes with multiple visitable base classes. Virtual visitable base classes are also supported. When visiting a derived object, the first class matching the visitor is used (starting from parent classes). Multiple and virtual inheritance is fully supported.

```cpp
// C is inherited from A (both can be visited)
struct C: public revisited::DerivedVisitable<C, A> { };
// D is inherited from A and B (A and B can be visited)
struct D: public revisited::JoinVisitable<A, B> { };
// E is virtually inherited from  A and B (E, A and B can be visited)
struct E: public revisited::DerivedVisitable<E, revisited::VirtualVisitable<A, B>> { };
```

### revisited::Any Examples

#### Implicit casting

```cpp
revisited::Any v;
v = 42;
std::cout << v.get<int>() << std::endl; // -> 42
std::cout << v.get<double>() << std::endl; // -> 42
v = "Hello Any!";
std::cout << v.get<std::string>() << std::endl; // -> Hello Any!
```

#### Reference aware casting

```cpp
int x = 42;
revisited::Any a = std::reference_wrapper(x);
std::cout << a.get<double>() << std::endl; // -> 42
std::cout << &a.get<int&>() == &x << std::endl; // -> 1
```

#### Inheritance aware casting

```cpp
// inheritance aware
struct MyClassBase{ int value; };
struct MyClass: public MyClassBase{ MyClass(int value):MyClassBase{value}{ } };
revisited::Any v;
v.setWithBases<MyClass, MyClassBase>(42);
std::cout << v.get<MyClassBase &>().value << std::endl; // -> 42
std::cout << v.get<MyClass &>().value << std::endl; // -> 42
```

### revisited::AnyFunction Examples

```cpp
revisited::AnyFunction f;
f = [](int x, float y){ return x + y; };
std::cout << f(40,2).get<int>() << std::endl; // -> 42
```

## Installation and usage

With [CPM](https://github.com/TheLartians/CPM), revisited::Visitor can be used in a CMake project simply by adding the following to the project's `CMakeLists.txt`.

```cmake
CPMAddPackage(
  NAME Revisited
  GIT_REPOSITORY https://github.com/TheLartians/Visitor.git
  VERSION 1.7
)

target_link_libraries(myProject Revisited)
```

Alternatively, the repository can be cloned locally and included it via `add_subdirectory`. Installing revisited::Visitor will make it findable in CMake's `find_package`.

## Performance

revisited::Visitor uses metaprogramming to determine the inheritance hierachy at compile-time for optimal performance. Compared to the traditional visitor pattern revisited::Visitor requires an additional virtual calls (as the type of the visitor and the visitable object are unknown). With compiler optimizations enabled, these calls should not be noticable in real-world applications.

There is an benchmark suite included in the repository that compares the pure cost of the different approaches.

```bash
git clone https://github.com/TheLartians/Visitor.git
cmake -HVisitor/benchmark -BVisitor/build/benchmark -DCMAKE_BUILD_TYPE=Release
cmake --build Visitor/build/benchmark -j8
./Visitor/build/benchmark/RevisitedBenchmark
```
