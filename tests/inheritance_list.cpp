
#include <catch2/catch.hpp>
#include <exception>

#include <lars/to_string.h>
#include <lars/inheritance_list.h>

template <typename ... Args> using T = lars::TypeList<Args...>;

TEST_CASE("InheritanceList") {
  
  struct A{};
  struct B{};
  struct C{};
  struct D{};
  struct E{};

  SECTION("Push and Merge"){
    using L = lars::InheritanceList<>;
    REQUIRE(std::is_same<L::Types, T<>>::value);

    using LA = L::Push<A, 1>;  // "{[A,1]}"
    REQUIRE(std::is_same<LA::Types, T<A>>::value);
    
    using LAB = LA::Push<B, 0>; // "{[A,1][B,0]}"
    REQUIRE(std::is_same<LAB::Types, T<A,B>>::value);
    
    using LAB2 = LAB::Push<B, 2>; // "{[B,2][A,1]}"
    REQUIRE(std::is_same<LAB2::Types, T<B,A>>::value);

    using LAB3 = LAB2::Push<B, 0>; // "{[B,2][A,1]}"
    REQUIRE(std::is_same<LAB3::Types, T<B,A>>::value);

    using LAB4 = LAB3::Push<B, 1>; // "{[B,2][A,1]}"
    REQUIRE(std::is_same<LAB4::Types, T<B,A>>::value);

    using LAB5 = LAB4::Push<A, 1>; // "{[B,2][A,1]}"
    REQUIRE(std::is_same<LAB5::Types, T<B,A>>::value);

    using LAB6 = LAB5::Push<A, 0>; // "{[B,2][A,1]}"
    REQUIRE(std::is_same<LAB6::Types, T<B,A>>::value);

    using LAB7 = LAB6::Push<A, 3>; // "{[A,3][B,2]}"
    REQUIRE(std::is_same<LAB7::Types, T<A,B>>::value);

    using LAB8 = LAB7::Push<C, 0>; // "{[A,3][B,2][C,0]}"
    REQUIRE(std::is_same<LAB8::Types, T<A,B,C>>::value);

    using LAB9 = LAB8::Push<D, 1>; // "{[A,3][B,2][D,1][C,0]}"
    REQUIRE(std::is_same<LAB9::Types, T<A,B,D,C>>::value);

    using LAB10 = LAB9::Push<D, 4>; // "{[D,4][A,3][B,2][C,0]}"
    REQUIRE(std::is_same<LAB10::Types, T<D,A,B,C>>::value);

    using ACE = lars::InheritanceList<>::Push<A, 5>::Push<C, 3>::Push<E,1>;
    using LAB11 = LAB10::Merge<ACE>; // "{[A,5][D,4][C,3][B,2][E,1]}"
    REQUIRE(std::is_same<LAB11::Types, T<A,D,C,B,E>>::value);
  }
  
  SECTION("Push"){
    using LABC = lars::InheritanceList<>::Push<A>::Push<B>::Push<C>; // "{[C,2][B,1][A,0]}"
    REQUIRE(std::is_same<LABC::Types, T<C,B,A>>::value);
  }
}
