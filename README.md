[![Build Status](https://travis-ci.com/TheLartians/Visitor.svg?branch=master)](https://travis-ci.com/TheLartians/Visitor)
[![codecov](https://codecov.io/gh/TheLartians/Visitor/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/Visitor)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/eb1f529643bd4e09a92c9dfc5b5920c4)](https://www.codacy.com/app/TheLartians/Visitor?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=TheLartians/Visitor&amp;utm_campaign=Badge_Grade)

# lars::Visitor

A C++17 visitor template and inheritance-aware any class. Using the acylclic visitor pattern greatly reduces the boilerplate code required for implementing the [visitor pattern](https://en.wikipedia.org/wiki/Visitor_pattern) in C++.

## Examples

See the [examples directory](https://github.com/TheLartians/Visitor/tree/master/examples) for full examples.

### Visitor Example

#### Simple Visitor

```cpp
#include <memory>
#include <iostream>
#include <lars/visitor.h>

struct Base: public virtual lars::VisitableBase { };
struct A: public Base, public lars::Visitable<A> { };
struct B: public Base, public lars::Visitable<B> { };

struct Visitor: public lars::Visitor<A &,B &> {
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

lars::Visitor also understands derived classes and classes with multiple visitable base classes. Virtual visitable base classes are also supported. When visiting a derived object, the first class matching the visitor is used (starting from parent classes). Multiple and virtual inheritance is fully supported.

```cpp
// C is inherited from A (both can be visited)
struct C: public lars::DerivedVisitable<C, A> { };
// D is inherited from A and B (A and B can be visited)
struct D: public lars::JoinVisitable<A, B> { };
// E is virtually inherited from  A and B (E, A and B can be visited)
struct E: public lars::DerivedVisitable<E, lars::VirtualVisitable<A, B>> { };
```

### Any Examples

#### Implicit casting

```cpp
lars::Any v = 42;
std::cout << v.get<int>() << std::endl; // -> 42
std::cout << v.get<double>() << std::endl; // -> 42
v = "Hello Any!";
std::cout << v.get<std::string>() << std::endl; // -> Hello Any!
```

#### Inheritance aware casting

```cpp
// inheritance aware
struct MyClassBase{ int value; };
struct MyClass: public MyClassBase{ MyClass(int value):MyClassBase{value}{ } };
lars::Any v;
v.setWithBases<MyClass, MyClassBase>(42);
std::cout << v.get<MyClassBase &>().value << std::endl; // -> 42
std::cout << v.get<MyClass &>().value << std::endl; // -> 42
```

## Installation and usage

With [CPM](https://github.com/TheLartians/CPM), lars::Visitor can be used in a CMake project simply by adding the following to the project's `CMakeLists.txt`.

```cmake
CPMAddPackage(
  NAME LarsVisitor
  GIT_REPOSITORY https://github.com/TheLartians/Visitor.git
  VERSION 0.9
)

target_link_libraries(myProject LarsVisitor)
```

Alternatively, the repository can be cloned locally and included it via `add_subdirectory`. Installing lars::Visitor will make it findable in CMake's `find_package`.

## Performance

Compared to the traditional visitor pattern lars::Visitor requires additional virtual calls, causing a slight performance impact. With compiler optimizations enabled, this impact should be hardly noticable. Using the visitor pattern to replace `dynamic_cast<T>` with `visitor_cast<T>` should even slightly improve performance.

The included benchmark suite compares the cost of the different approaches.

```bash
git clone https://github.com/TheLartians/Visitor.git
cmake -HVisitor/benchmark -BVisitor/build/benchmark -DCMAKE_BUILD_TYPE=Release
cmake --build Visitor/build/benchmark -j
./Visitor/build/benchmark/LarsVisitorBenchmark
```
