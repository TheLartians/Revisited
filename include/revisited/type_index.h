#pragma once

#include <functional>
#include <ostream>
#include <static_type_info.h>

namespace revisited {
using StaticTypeIndex = static_type_info::TypeID;

struct TypeIndex : public static_type_info::TypeID {};

inline std::ostream &operator<<(std::ostream &stream, const TypeIndex &idx) {
  stream << idx.name;
  return stream;
}

template <class T> constexpr StaticTypeIndex getStaticTypeIndex() {
  return static_type_info::getTypeID<T>();
}

template <class T> TypeIndex getTypeIndex() {
  return TypeIndex{static_type_info::getTypeID<T>()};
}

template <class T> auto get_type_name() {
  return static_type_info::getTypeName<T>();
}

} // namespace revisited

template <> struct std::hash<revisited::TypeIndex> {
  std::size_t operator()(const revisited::TypeIndex &t) const {
    return t.index;
  }
};
