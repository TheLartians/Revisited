#pragma once

#define CTTI

#ifdef CTTI

#include <ctti/type_id.hpp>

namespace lars{
  
  using TypeIndex = ctti::type_index;
  
  template <class T> constexpr TypeIndex get_type_index(){
    return ctti::type_id<T>();
  }
  
  template <class T> std::string get_type_name(){
    auto name = ctti::type_id<T>().name();
    return std::string(name.begin(),name.end());
  }
  
}

#else

namespace lars{
  
  using TypeIndex = unsigned;

  template <int CONTEXT = 0> struct TypeIndexContext{
    static lars::TypeIndex type_count;
    template <class T> struct Index{ static lars::TypeIndex value; };
  };
  
  template <int CONTEXT> TypeIndex TypeIndexContext<CONTEXT>::type_count = 0;
  template <int CONTEXT> template <class T> TypeIndex TypeIndexContext<CONTEXT>::Index<T>::value = TypeIndexContext<CONTEXT>::type_count++;
  
  template <class T> TypeIndex get_type_index(){
    return TypeIndexContext<>::Index<T>::value;
  }

}

#endif
