
#include <catch2/catch.hpp>
#include <exception>

#include <lars/to_string.h>
#include <lars/visitor/inheritance_list.h>


TEST_CASE("InheritanceList") {
  
  struct A{};
  struct B{};
  struct C{};
  struct D{};

  using L = lars::InheritanceList<>;
  using LA = L::Push<A, 0>;
  using LAB = LA::Push<B, 0>;
  REQUIRE(lars::stream_to_string(LAB()) == "{[A,0][B,0]}");
  
  using LABB = LAB::Push<B, 1>;
  REQUIRE(lars::stream_to_string(LABB()) == "{[B,1][A,0]}");

};
