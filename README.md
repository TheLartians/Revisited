[![Build Status](https://travis-ci.com/TheLartians/Visitor.svg?branch=master)](https://travis-ci.com/TheLartians/Visitor)
[![codecov](https://codecov.io/gh/TheLartians/Visitor/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/Visitor)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/eb1f529643bd4e09a92c9dfc5b5920c4)](https://www.codacy.com/app/TheLartians/Visitor?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=TheLartians/Visitor&amp;utm_campaign=Badge_Grade)

# lars::Visitor

A C++17 visitor template and inheritance-aware any class.

## Examples

See the [examples directory](https://github.com/TheLartians/Visitor/tree/master/examples) for full examples.

### Visitor Example

```cpp
#include <memory>
#include <iostream>
#include <lars/visitor.h>

struct Base: public virtual lars::VisitableBase{ };
struct A: public Base, public lars::Visitable<A> { };
struct B: public Base, public lars::Visitable<B> { };

struct Visitor: public lars::Visitor<A &,B &> {
  void visit(A &){ std::cout << "Visiting A" << std::endl; }
  void visit(B &){ std::cout << "Visiting B" << std::endl; }
};

int main() {
  std::shared_ptr<Base> a = std::make_shared<A>();
  std::shared_ptr<Base> b = std::make_shared<B>();
  
  ABVisitor abVisitor;
  a->accept(abVisitor); // -> Visiting A
  b->accept(abVisitor); // -> Visiting B
}
```

### Any Example

```cpp
lars::Any v = 42;
std::cout << v.get<int>() << std::endl; // -> 42
std::cout << v.get<double>() << std::endl; // -> 42
v = "Hello Any!";
std::cout << v.get<std::string>() << std::endl; // -> Hello Any!
```

## Installation and usage

With [CPM](https://github.com/TheLartians/CPM), lars::Visitor can be used in a CMake project simply by adding the following to the project's `CMakeLists.txt`.

```cmake
CPMAddPackage(
  NAME LarsVisitor
  GIT_REPOSITORY https://github.com/TheLartians/Visitor.git
  VERSION 0.8
)

target_link_libraries(myProject LarsVisitor)
```

Alternatively, the repository can be cloned locally and included it via `add_subdirectory`. Installing lars::Visitor will make it findable in CMake's `find_package`.
