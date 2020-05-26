#pragma once

#include <static_type_info.h>

#include <functional>

namespace revisited {
  using TypeIndex = static_type_info::TypeIndex;

  struct TypeID : public static_type_info::TypeID {};

  template <class T> constexpr TypeIndex getTypeIndex() {
    return static_type_info::getTypeIndex<T>();
  }

  template <class T> TypeID getTypeID() { return TypeID{static_type_info::getTypeID<T>()}; }

}  // namespace revisited

template <> struct std::hash<revisited::TypeID> {
  hash<static_type_info::TypeID> hasher;
  std::size_t operator()(const revisited::TypeID &t) const { return hasher(t); }
};
