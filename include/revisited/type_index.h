#pragma once

#include <ctti/type_id.hpp>
#include <functional>

namespace revisited {

class StaticTypeIndex {
private:
  ctti::detail::hash_t _hash;

public:
  constexpr StaticTypeIndex(const ctti::type_id_t &t) : _hash(t.hash()) {}
  constexpr bool operator==(const StaticTypeIndex &other) const {
    return other._hash == _hash;
  }
  constexpr bool operator!=(const StaticTypeIndex &other) const {
    return other._hash != _hash;
  }
  constexpr size_t hash() const { return _hash; }
};

class TypeIndex : public StaticTypeIndex {
private:
  ctti::type_id_t type_index;

public:
  TypeIndex(ctti::type_id_t &&t) : StaticTypeIndex(t), type_index(t) {}
  std::string name() const { return type_index.name().cppstring(); }
};

template <class Ostream>
Ostream &operator<<(Ostream &stream, const TypeIndex &idx) {
  stream << idx.name();
  return stream;
}

template <class T> struct StaticTypeIndexFor {
  static constexpr StaticTypeIndex value = StaticTypeIndex(ctti::type_id<T>());
};

template <class T> constexpr StaticTypeIndex getStaticTypeIndex() {
  return StaticTypeIndexFor<T>::value;
}

template <class T> TypeIndex getTypeIndex() {
  return TypeIndex(ctti::type_id<T>());
}

template <class T> std::string get_type_name() {
  auto name = ctti::type_id<T>().name();
  return std::string(name.begin(), name.end());
}

} // namespace revisited

template <> struct std::hash<revisited::StaticTypeIndex> {
  std::size_t operator()(const revisited::StaticTypeIndex &t) const {
    return t.hash();
  }
};

template <> struct std::hash<revisited::TypeIndex> {
  std::size_t operator()(const revisited::StaticTypeIndex &t) const {
    return t.hash();
  }
};
