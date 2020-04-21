#pragma once

#include <memory>
#include <revisited/visitor.h>

namespace revisited {

template <class T, class V>
std::shared_ptr<T> visitor_pointer_cast(const std::shared_ptr<V> &v) {
  if (auto res = visitor_cast<T *>(v.get())) {
    return std::shared_ptr<T>(v, res);
  } else {
    return std::shared_ptr<T>();
  }
}

} // namespace revisited
