#pragma once

#include <functional>
#include <static_type_info.h>

namespace revisited {
using TypeIndex = static_type_info::TypeID;

struct TypeID : public static_type_info::TypeID {};

template <class T> constexpr TypeIndex getTypeIndex() {
  return static_type_info::getTypeID<T>();
}

template <class T> TypeID getTypeID() {
  return TypeID{static_type_info::getTypeID<T>()};
}

} // namespace revisited

template <> struct std::hash<revisited::TypeID> {
  std::size_t operator()(const revisited::TypeID &t) const { return t.index; }
};
