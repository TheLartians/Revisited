
#include <catch2/catch.hpp>
#include <exception>

#include <lars/inheritance_list.h>

template <class T, unsigned V> using O = lars::OrderedType<T, V>;
template <typename ... Args> using T = lars::InheritanceList<Args...>;

TEST_CASE("InheritanceList") {
  
  struct A{};
  struct B{};
  struct C{};
  struct D{};
  struct E{};

  SECTION("Push and Merge"){
    using L = lars::InheritanceList<>;
    REQUIRE(std::is_same<L, T<>>::value);

    using LA = L::Push<A, 1>;
    REQUIRE(std::is_same<LA, T<O<A,1>>>::value);
    
    using LAB = LA::Push<B, 0>;
    REQUIRE(std::is_same<LAB, T<O<A,1>,O<B,0>>>::value);
    
    using LAB2 = LAB::Push<B, 2>;
    REQUIRE(std::is_same<LAB2, T<O<B,2>,O<A,1>>>::value);

    using LAB3 = LAB2::Push<B, 0>;
    REQUIRE(std::is_same<LAB3, T<O<B,2>,O<A,1>>>::value);

    using LAB4 = LAB3::Push<B, 1>;
    REQUIRE(std::is_same<LAB4, T<O<B,2>,O<A,1>>>::value);

    using LAB5 = LAB4::Push<A, 1>;
    REQUIRE(std::is_same<LAB5, T<O<B,2>,O<A,1>>>::value);

    using LAB6 = LAB5::Push<A, 0>;
    REQUIRE(std::is_same<LAB6, T<O<B,2>,O<A,1>>>::value);

    using LAB7 = LAB6::Push<A, 3>;
    REQUIRE(std::is_same<LAB7, T<O<A,3>,O<B,2>>>::value);

    using LAB8 = LAB7::Push<C, 0>;
    REQUIRE(std::is_same<LAB8, T<O<A,3>,O<B,2>,O<C,0>>>::value);

    using LAB9 = LAB8::Push<D, 1>;
    REQUIRE(std::is_same<LAB9, T<O<A,3>,O<B,2>,O<D,1>,O<C,0>>>::value);

    using LAB10 = LAB9::Push<D, 4>;
    REQUIRE(std::is_same<LAB10, T<O<D,4>,O<A,3>,O<B,2>,O<C,0>>>::value);

    using ACE = lars::InheritanceList<>::Push<A, 5>::Push<C, 3>::Push<E,1>;
    using LAB11 = LAB10::Merge<ACE>;
    REQUIRE(std::is_same<LAB11, T<O<A,5>,O<D,4>,O<C,3>,O<B,2>,O<E,1>>>::value);
  }
  
  SECTION("Push"){
    using LABC = lars::InheritanceList<>::Push<A>::Push<B>::Push<C>;
    REQUIRE(std::is_same<LABC, T<O<C,2>,O<B,1>,O<A,0>>>::value);
  }
}
