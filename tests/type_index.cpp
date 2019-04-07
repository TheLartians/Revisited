#include <catch2/catch.hpp>
#include <lars/type_index.h>

TEST_CASE("Type Index") {

  using namespace lars;

  static_assert(get_type_index<int>() == get_type_index<int>());
  static_assert(get_type_index<float>() == get_type_index<float>());
  static_assert(get_type_index<int>() != get_type_index<float>());

  using A = int;
  using B = float;

  static_assert(get_type_index<A>() == get_type_index<int>());
  static_assert(get_type_index<B>() == get_type_index<float>());
  static_assert(get_type_index<A>() != get_type_index<B>());
  static_assert(get_type_index<A>() != get_type_index<float>());
  static_assert(get_type_index<B>() != get_type_index<int>());
  
}
