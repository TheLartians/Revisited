#pragma once

#include <type_traits>

namespace revisited {

namespace typelist {
template <typename... Args> struct Merge;
template <class L, template <class> typename F> struct Filter;
} // namespace typelist

template <typename... Types> struct TypeList {
  template <typename... Other> using Push = TypeList<Types..., Other...>;
  template <typename... Other>
  using Merge = typename typelist::Merge<TypeList, Other...>::type;
  template <template <class> typename F>
  using Filter = typename typelist::Filter<TypeList, F>::type;
  template <template <class> typename T>
  using Transform = TypeList<typename T<Types>::type...>;
  template <class F, template <class> typename T>
  static constexpr auto transform(F &&f) {
    return f(T<Types>()...);
  }
};

namespace typelist {

template <typename... ATypes, typename... BTypes>
struct Merge<TypeList<ATypes...>, TypeList<BTypes...>> {
  using type = TypeList<ATypes..., BTypes...>;
};

template <class A, class B, typename... Rest> struct Merge<A, B, Rest...> {
  using type = typename Merge<typename Merge<A, B>::type, Rest...>::type;
};

template <template <class> typename F> struct Filter<TypeList<>, F> {
  using type = TypeList<>;
};

template <class First, typename... Rest, template <class> typename F>
struct Filter<TypeList<First, Rest...>, F> {
private:
  using RestType = typename Filter<TypeList<Rest...>, F>::type;

public:
  using type = typename std::conditional<
      F<First>::value, typename TypeList<First>::template Merge<RestType>,
      RestType>::type;
};

} // namespace typelist

} // namespace revisited