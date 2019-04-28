#pragma once

#include <type_traits>

#include <lars/type_list.h>
#include <lars/type_index.h>

namespace lars {
  
  template <class T, unsigned O> struct OrderedType {
    using type = T;
    const static unsigned value = O;
  };
  
  template <class OStream, class T, unsigned O> OStream & operator<<(OStream &stream, const OrderedType<T,O> &){
    stream << '[' << lars::get_type_name<T>() << ',' << O << ']';
    return stream;
  }

  template <typename ... OrderedTypes> struct InheritanceList;
  
  template <class T, class Previous, class List> struct InheritanceListRemover;
  template <class T, typename ... Previous> struct InheritanceListRemover <
    T,
    InheritanceList<Previous...>,
    InheritanceList<>
  > {
    using type = InheritanceList<Previous...>;
    const static unsigned value = 0;
  };
  template <class T, typename ... Previous, class FT, unsigned FO, typename ... Rest> struct InheritanceListRemover <
    T,
    InheritanceList<Previous...>,
    InheritanceList<OrderedType<FT, FO>, Rest ...>
  > {
  private:
    using RemoverType = typename std::conditional<
      std::is_same<T, FT>::value,
      InheritanceListRemover<
        T,
        InheritanceList<Previous..., Rest...>,
        InheritanceList<>
      >,
      InheritanceListRemover<
        T,
        InheritanceList<Previous..., OrderedType<FT, FO>>,
        InheritanceList<Rest...>
      >
    >::type;
    public:
    using type = typename RemoverType::type;
    const static unsigned value = std::is_same<T, FT>::value ? FO : RemoverType::value;
  };

  
  template <class T, unsigned O, class Previous, class List> struct InheritanceListPusher;
  
  template <class T, unsigned O, typename ... Previous> struct InheritanceListPusher <
    T,
    O,
    InheritanceList<Previous...>,
    InheritanceList<>
  > {
    using type = InheritanceList<Previous..., OrderedType<T, O>>;
  };
  
  template <typename ... Args> struct InheritanceListMerger;
  template <class A, class B, typename ... Rest> struct InheritanceListMerger<A,B,Rest...>{
    using type = typename InheritanceListMerger<
      typename InheritanceListMerger<A,B>::type,
      Rest...
    >::type;
  };
  template <class B> struct InheritanceListMerger<
    InheritanceList<>,
    B
  > {
    using type = B;
  };
  template <typename ... ARest, class FT, unsigned FO, class B> struct InheritanceListMerger<
    InheritanceList<OrderedType<FT, FO>, ARest...>,
    B
  > {
    using type = typename InheritanceListMerger<
      InheritanceList<ARest...>,
      typename B::template Push<FT, FO>
    >::type;
  };
  
  template <class T, unsigned O, typename ... Previous, class FT, unsigned FO, typename ... Rest> struct InheritanceListPusher <
    T,
    O,
    InheritanceList<Previous ...>,
    InheritanceList<OrderedType<FT, FO>, Rest ...>
  > {
    using type = typename std::conditional<
      (O >= FO),
      InheritanceList<Previous..., OrderedType<T, O>, OrderedType<FT, FO>, Rest...>,
      typename InheritanceListPusher<
        T,
        O,
        InheritanceList<Previous..., OrderedType<FT, FO>>,
        InheritanceList<Rest...>
      >::type
    >::type;
  };
  
  template <class A> struct InheritanceListNextOrder;
  template <> struct InheritanceListNextOrder<InheritanceList<>> {
    static const unsigned value = 0;
  };
  template <class FT, unsigned FO, typename ... Rest> struct InheritanceListNextOrder<
    InheritanceList<OrderedType<FT, FO>, Rest...>
  > {
    static const unsigned value = FO+1;
  };

  template <typename ... OrderedTypes> struct InheritanceList {
  private:
    template <class T> using Removed = InheritanceListRemover<T, InheritanceList<>, InheritanceList<OrderedTypes...>>;

  public:
    const static size_t size = sizeof...(OrderedTypes);
    
    template <typename ... O> using Merge = typename InheritanceListMerger<InheritanceList, O...>::type;
    
    template <class T, unsigned O = InheritanceListNextOrder<InheritanceList>::value> using Push = typename InheritanceListPusher<
      T,
      std::max(Removed<T>::value, O),
      InheritanceList<>,
      typename Removed<T>::type
    >::type;
    
    using Types = TypeList<typename OrderedTypes::type ...>;
    using ConstTypes = TypeList<const typename OrderedTypes::type ...>;
    using ReferenceTypes = TypeList<typename OrderedTypes::type &...>;
    using ConstReferenceTypes = TypeList<const typename OrderedTypes::type &...>;
    
    using ConvertibleTypes = TypeList<>::template Merge<ReferenceTypes,ConstReferenceTypes>;
    using ConstConvertibleTypes = ConstReferenceTypes;
  };
  
  template <class OStream, typename ... Types> OStream & operator<<(OStream &stream, const InheritanceList<Types...> &){
    stream << '{';
    auto forEach = [](auto...){};
    auto print = [&](auto t){ stream << t; return 0; };
    forEach(print(Types()) ...);
    stream << '}';
    return stream;
  }
  
}
