#include <catch2/catch.hpp>
#include <lars/any.h>

namespace{
  struct A{ };
  struct B:public A{ };
  struct C:public B{ };
  struct D{ };
  struct E: public C, public D{ };
}

TEST_CASE("Any") {
  lars::Any v;
  
  v.set<int, double, float, char, size_t, unsigned>(4);
  
}
