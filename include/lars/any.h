
#include <lars/visitor.h>

#include <string>
#include <memory>

namespace lars {
  
  template <class T> struct AnyVisitable;
  
  template <class T> struct AnyVisitable {
    using type = typename std::conditional<
      std::is_base_of<VisitableBase, T>::value,
      T,
      DataVisitable<T, TypeList<T &>, TypeList<const T &, T>>
    >::type;
  };
  
  class Any {
  private:
    std::unique_ptr<VisitableBase> data;
    
  public:
    
    Any():data(std::make_unique<EmptyVisitable>()){}
    template <class T> Any(T && v){ set<typename std::remove_reference<T>::type>(v); }
    Any(const Any &) = delete;
    Any(Any &&) = default;
    Any &operator=(const Any &) = delete;
    Any &operator=(Any &&) = default;

    template <
      class T,
      class VisitableType = typename AnyVisitable<T>::type,
      typename ... Args
    > void set(Args && ... args) {
      data = std::make_unique<VisitableType>(std::forward<Args>(args)...);
    }
        
    template <class T> Any & operator=(T && o) {
      set<typename std::remove_reference<T>::type>(o);
      return *this;
    }
    
    template <class T> T get(){
      return visitor_cast<T>(*data);
    }
    
    template <class T> const T get()const{
      return visitor_cast<T>(*data);
    }
    
  };
  
}

/**
 * Any conversions
 */

#define LARS_ANY_DEFINE_SCALAR_TYPE(Type,Conversions) \
template <> struct lars::AnyVisitable<Type>{\
  using Types = typename TypeList<Type &>::template Merge<Conversions>; \
  using ConstTypes = typename TypeList<const Type &>::template Merge<Conversions>; \
  using type = DataVisitable<Type, Types, ConstTypes>; \
}

#ifndef LARS_ANY_NUMERIC_TYPES
#define LARS_ANY_NUMERIC_TYPES ::lars::TypeList<char, int, long, long long, unsigned char, unsigned int, unsigned long, unsigned long long, float, double>
#endif

LARS_ANY_DEFINE_SCALAR_TYPE(char, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(int, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(long, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(long long, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(unsigned char, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(unsigned int, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(unsigned long, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(unsigned long long, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(float, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(double, LARS_ANY_NUMERIC_TYPES);
LARS_ANY_DEFINE_SCALAR_TYPE(long double, LARS_ANY_NUMERIC_TYPES);

template <size_t N> struct lars::AnyVisitable<char[N]> {
  using type = AnyVisitable<std::string>::type;
};

template <size_t N> struct lars::AnyVisitable<const char[N]> {
  using type = AnyVisitable<std::string>::type;
};
