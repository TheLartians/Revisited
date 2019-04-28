#pragma once

#include <lars/visitor.h>

#include <string>
#include <memory>
#include <exception>
#include <functional>
#include <utility>
#include <ostream>

namespace lars {
  /**
   * Error raised when calling `get` on empty Any object.
   */
  struct UndefinedAnyException: public std::exception{
    const char * what() const noexcept override {
      return "called get() on undefined Any";
    }
  };
  
  template <class T> struct AnyVisitable;
  struct AnyReference;

  /**
   * A class that can hold an arbitrary value of any type.
   */
  class Any {
  protected:
    std::shared_ptr<VisitableBase> data;
    
  public:
    
    Any(){}
    template <
      class T,
      typename = typename std::enable_if<!std::is_base_of<Any,T>::value>::type
    > Any(T && v){ set<typename std::remove_reference<T>::type>(v); }
    Any(const Any &) = delete;
    Any(Any &&) = default;
    Any &operator=(const Any &) = delete;
    Any &operator=(Any &&) = default;
    
    template <
      class T,
      typename = typename std::enable_if<!std::is_base_of<Any,T>::value>::type
    > Any & operator=(T && o) {
      set<typename std::remove_reference<T>::type>(o);
      return *this;
    }
    
    /**
     * Sets the stored object to an object of type `T`, constructed with the arguments provided.
     * The `VisitableType` templated paramter defines the internal type used for storing and
     * casting the object. The default is `lars::AnyVisitable<T>::type` which can be specialized
     * for usertypes.
     */
    template <
      class T,
      class VisitableType = typename AnyVisitable<T>::type,
      typename ... Args
    > void set(Args && ... args) {
      static_assert(!std::is_base_of<Any,T>::value);
      data = std::make_shared<VisitableType>(std::forward<Args>(args)...);
    }
    
    /**
     * Same as `Any::set`, but uses an internal type that can be visitor_casted to the base types.
     */
    template <class T, typename ... Bases, typename ... Args> void setWithBases(Args && ... args){
      set<T, DataVisitableWithBases<T,Bases...>>(std::forward<Args>(args)...);
    }
    
    /**
     * Captures the value from another any object
     */
    void setReference(const Any & other){
      data = other.data;
    }
    
    /**
     * Casts the internal data to `T` using `visitor_cast`.
     * A `InvalidVisitor` exception will be raised if the cast is unsuccessful.
     */
    template <class T> T get() const {
      if (!data) { throw UndefinedAnyException(); }
      return visitor_cast<T>(*data);
    }

    /**
     * Casts the internal data to `T *` using `visitor_cast`.
     * `nullptr` will be returned if the cast is unsuccessful.
     */
    template <class T> T * tryGet() const {
      if(!data) { return nullptr; }
      return visitor_cast<T*>(data.get());
    }
    
    /**
     * `true`, when contains value, `false` otherwise
     */
    operator bool()const{
      return bool(data);
    }
    
    /**
     * resets the value
     */
    void reset(){
      data.reset();
    }
    
    /**
     * the type of the stored value
     */
    TypeIndex type()const{
      if (!data) { return lars::getTypeIndex<void>(); }
      return data->StaticTypeIndex();
    }

    /**
     * Accept visitor
     */
    void accept(VisitorBase &visitor){
      if (!data) { throw UndefinedAnyException(); }
      data->accept(visitor);
    }

    void accept(VisitorBase &visitor) const {
      if (!data) { throw UndefinedAnyException(); }
      std::as_const(*data).accept(visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor){
      if (!data) { throw UndefinedAnyException(); }
      return data->accept(visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const {
      if (!data) { throw UndefinedAnyException(); }
      return std::as_const(*data).accept(visitor);
    }
    
  };

  template <class T, typename ... Args> Any make_any(Args && ... args){
    Any v;
    v.set<T>(std::forward<Args>(args)...);
    return v;
  }
  
  inline std::ostream &operator<<(std::ostream &stream, const Any &v){
    stream << "Any<" << v.type() << ">";
    return stream;
  }
  
  /**
   * An Any object that can implicitly capture another Any by reference.
   */
  struct AnyReference: public Any {
    using Any::Any;
    AnyReference(const Any &other){ setReference(other); }
    AnyReference(const AnyReference &other):Any(){ setReference(other); }
    
    AnyReference &operator=(const Any &other){
      setReference(other);
      return *this;
    }
    
    template <class T> typename std::enable_if<std::is_same<const Any &, T>::value, T>::type get() const {
      return *this;
    }
    
    template <class T> typename std::enable_if<!std::is_same<const Any &, T>::value, T>::type get() const {
      if (!data) { throw UndefinedAnyException(); }
      return visitor_cast<T>(*data);
    }
    
  };
    
  /**
   * Defines the default internal type for Any<T>.
   * Specialize this class to support implicit conversions usertypes.
   */
  template <class T> struct AnyVisitable {
    using type = typename std::conditional<
      std::is_base_of<VisitableBase, T>::value,
      T,
      DataVisitable<T>
    >::type;
  };

}

/**
 * Numeric any conversions.
 */
#define LARS_ANY_DEFINE_SCALAR_TYPE(Type,Conversions) \
template <> struct lars::AnyVisitable<Type>{\
  using Types = typename lars::TypeList<Type &, const Type &>::template Merge<Conversions>; \
  using ConstTypes = typename lars::TypeList<const Type &>::template Merge<Conversions>; \
  using type = lars::DataVisitablePrototype<Type, Types, ConstTypes>; \
}

#ifndef LARS_ANY_NUMERIC_TYPES
#define LARS_ANY_NUMERIC_TYPES ::lars::TypeList<char, int, long, long long, unsigned char, unsigned int, unsigned long, unsigned long long, float, double, long double>
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

/**
 * Char arrays are captured as strings.
 */
template <size_t N> struct lars::AnyVisitable<char[N]> {
  using type = lars::AnyVisitable<std::string>::type;
};

template <size_t N> struct lars::AnyVisitable<const char[N]> {
  using type = lars::AnyVisitable<std::string>::type;
};

/**
 * Capture values as reference through `std::reference_wrapper`.
 */
template <class T> struct lars::AnyVisitable<std::reference_wrapper<T>> {
  using type = lars::DataVisitablePrototype<
    std::reference_wrapper<T>,
    typename AnyVisitable<T>::type::Types,
    typename AnyVisitable<T>::type::ConstTypes,
    T
  >;
};
