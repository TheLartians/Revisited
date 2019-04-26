#pragma once

#include <ctti/type_id.hpp>
#include <functional>

namespace lars{
  
  class TypeIndex {
  private:
    ctti::detail::hash_t _hash;
  public:
    constexpr TypeIndex(const ctti::type_id_t& t):_hash(t.hash()){ }
    constexpr bool operator==(const TypeIndex &other)const{ return other._hash == _hash; }
    constexpr bool operator!=(const TypeIndex &other)const{ return other._hash != _hash; }
    constexpr size_t hash() const { return _hash; }
  };
  
  class NamedTypeIndex: public TypeIndex {
  private:
    ctti::type_id_t type_index;
  public:
    NamedTypeIndex(ctti::type_id_t && t):TypeIndex(t),type_index(t){ }
    std::string name() const { return type_index.name().cppstring(); }
  };
  
  template <class T> struct GetTypeIndex {
    static constexpr TypeIndex value = TypeIndex(ctti::type_id<T>());
  };
  
  template <class T> constexpr TypeIndex getTypeIndex(){
    return GetTypeIndex<T>::value;
  }
  
  template <class T> NamedTypeIndex getNamedTypeIndex(){
    return NamedTypeIndex(ctti::type_id<T>());
  }

  template <class T> std::string get_type_name(){
    auto name = ctti::type_id<T>().name();
    return std::string(name.begin(),name.end());
  }
  
}

namespace std{
  
  template <> struct hash<lars::TypeIndex> {
    std::size_t operator()(const lars::TypeIndex& t) const {
      return t.hash();
    }
  };
  
}
