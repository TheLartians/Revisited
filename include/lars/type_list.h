#pragma once

#include <type_traits>

namespace lars{

  namespace typelist {
    template <typename ... Args> struct Merge;
    template <class L, template <class> typename F > struct Filter;
  }

  template <typename ... Args> struct TypeList {
    template <typename ... Types> using Push = TypeList<Args..., Types...>;
    template <typename ... Other> using Merge = typename typelist::Merge<TypeList, Other...>::type;
    template <template <class> typename Filter> using Filter = typename typelist::Filter<TypeList, Filter>::type;
    template <template <class> typename T> using Transform = TypeList<typename T<Args>::type ...>;
  };

  namespace typelist {

    template <typename ... ATypes, typename ... BTypes> struct Merge <TypeList<ATypes...>, TypeList<BTypes...>> {
      using type = TypeList<ATypes..., BTypes...>;
    };

    template <class A, class B, typename ... Rest> struct Merge <A, B, Rest...> {
      using type = typename Merge<typename Merge<A,B>::type, Rest...>::type;
    };

    template < template <class> typename F> struct Filter<TypeList<>, F> {
      using type = TypeList<>;
    };

    template <class First, typename ... Rest, template <class> typename F> struct Filter<TypeList<First, Rest...>, F> {
    private:
      using RestType = typename Filter<TypeList<Rest...>, F>::type;
    public:
      using type = typename std::conditional<
        F<First>::value,
        typename TypeList<First>::template Merge<RestType>,
        RestType
      >::type;
    };
    
  }

}