#pragma once

#include <ctti/type_id.hpp>

namespace lars{
  
  using TypeIndex = ctti::type_id_t;

  template <class T> constexpr TypeIndex get_type_index(){
    return ctti::type_id<T>();
  }
  
  template <class T> std::string get_type_name(){
    auto name = ctti::type_id<T>().name();
    return std::string(name.begin(),name.end());
  }
  
}
