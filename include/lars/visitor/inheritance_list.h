#pragma once

#include <ostream>
#include <type_traits>

#include <lars/type_index.h>
#include <lars/dummy.h>

namespace lars {
  
  template <class T, int O> struct OrderedType {
    using type = T;
    const static int value = O;
  };
  
  template <class T, int O> std::ostream & operator<<(std::ostream &stream, const OrderedType<T,O> &){
    stream << '[' << lars::get_type_name<T>() << ',' << O << ']';
    return stream;
  }

  
  template <typename ... OrderedTypes> struct InheritanceList;
  
  template <> struct InheritanceList<> {
    const static size_t size = 0;
    template <class T, int O> using Push = InheritanceList<OrderedType<T, O>>;
  };
  
  template <typename ... Types> struct InheritanceList {
    const static size_t size = sizeof...(Types);
    template <class T, int O> using Push = InheritanceList<Types..., OrderedType<T, O>>;
  };
  
  template <typename ... Types> std::ostream & operator<<(std::ostream &stream, const InheritanceList<Types...> &){
    stream << '{';
    auto print = [&](auto t){ stream << t; return 0; };
    lars::dummy_function(print(Types()) ...);
    stream << '}';
    return stream;
  }
  
}
