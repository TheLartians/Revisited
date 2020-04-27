#pragma once

#include <revisited/visitor.h>

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace revisited {
  /**
   * Error raised when calling `get` on empty Any object.
   */
  struct UndefinedAnyException : public std::exception {
    const char *what() const noexcept override { return "accessed data on undefined Any"; }
  };

  template <class T> struct AnyVisitable;

  class Any;

  namespace any_detail {
    template <typename T> struct is_shared_ptr : std::false_type { using value_type = void; };
    template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {
      using value_type = T;
    };
    template <class T> struct remove_cvref {
      typedef std::remove_cv_t<std::remove_reference_t<T>> type;
    };

    template <class T> constexpr static bool NotDerivedFromAny
        = !std::is_base_of<Any, typename std::decay<T>::type>::value;

    template <class T> struct CapturedSharedPtr : public std::shared_ptr<T> {
      CapturedSharedPtr(const std::shared_ptr<T> &d) : std::shared_ptr<T>(d) {}
      operator T &() { return **this; }
      operator const T &() const { return **this; }
    };
  }  // namespace any_detail

  /**
   * A class that can hold an arbitrary value of any type.
   */
  class Any {
  protected:
    std::shared_ptr<VisitableBase> data;

  public:
    Any() {}
    Any(Any &&) = default;
    Any(const Any &) = default;
    Any &operator=(Any &&) = default;
    Any &operator=(const Any &) = default;

    template <class T, typename = typename std::enable_if<any_detail::NotDerivedFromAny<T>>::type>
    Any(T &&v) {
      set<typename any_detail::remove_cvref<T>::type>(std::forward<T>(v));
    }

    template <class T, typename = typename std::enable_if<
                           !std::is_base_of<Any, typename std::decay<T>::type>::value>::type>
    Any &operator=(T &&o) {
      set<typename any_detail::remove_cvref<T>::type>(std::forward<T>(o));
      return *this;
    }

    /**
     * Sets the stored object to an object of type `T`, constructed with the
     * arguments provided. The `VisitableType` templated paramter defines the
     * internal type used for storing and casting the object. The default is
     * `revisited::AnyVisitable<T>::type` which can be specialized for usertypes.
     */
    template <class T, class VisitableType = typename AnyVisitable<T>::type, typename... Args>
    decltype(auto) set(Args &&... args) {
      static_assert(!std::is_base_of<Any, T>::value);
      if constexpr (std::is_base_of<VisitableBase,
                                    typename any_detail::is_shared_ptr<T>::value_type>::value) {
        T value(std::forward<Args>(args)...);
        data = value;
        if constexpr (any_detail::is_shared_ptr<T>::value) {
          if (!value) {
            data.reset();
          }
        }
      } else if constexpr (any_detail::is_shared_ptr<T>::value) {
        T value(std::forward<Args>(args)...);
        if (!value) {
          data.reset();
        } else {
          data = std::make_shared<VisitableType>(value);
        }
      } else {
        auto value = std::make_shared<VisitableType>(std::forward<Args>(args)...);
        data = value;
        return static_cast<typename VisitableType::Type &>(*value);
      }
    }

    /**
     * Same as `Any::set`, but uses an internal type that can be visitor_casted to
     * the base types.
     */
    template <class T, typename... Bases, typename... Args>
    decltype(auto) setWithBases(Args &&... args) {
      return set<T, DataVisitableWithBases<T, Bases...>>(std::forward<Args>(args)...);
    }

    /**
     * Captures the value from another any object
     */
    void setReference(const Any &other) { data = other.data; }

    /**
     * Casts the internal data to `T` using `visitor_cast`.
     * returns std::nullopt if the cast is unsuccessful.
     * if `T` is a reference type, a pointer will be returned.
     */
    template <class T>
    typename std::enable_if<!std::is_reference<T>::value, std::optional<T>>::type as() const {
      if constexpr (std::is_same<typename std::decay<T>::type, Any>::value) {
        return *this;
      } else if constexpr (any_detail::is_shared_ptr<T>::value) {
        using Value = typename any_detail::is_shared_ptr<T>::value_type;
        if (!data) {
          return std::nullopt;
        }
        return std::shared_ptr<Value>(data, tryGet<Value>());
      } else {
        if (!data) {
          return std::nullopt;
        }
        return opt_visitor_cast<T>(*data);
      }
    }

    template <class T> typename std::enable_if<std::is_reference<T>::value,
                                               typename std::remove_reference<T>::type *>::type
    as() const {
      return tryGet<typename std::remove_reference<T>::type>();
    }

    /**
     * Casts the internal data to `T` using `visitor_cast`.
     * A `InvalidVisitor` exception will be raised if the cast is unsuccessful.
     */
    template <class T> T get() const {
      if constexpr (std::is_same<typename std::decay<T>::type, Any>::value) {
        return *this;
      } else if constexpr (any_detail::is_shared_ptr<T>::value) {
        using Value = typename any_detail::is_shared_ptr<T>::value_type;
        if (!data) {
          throw UndefinedAnyException();
        }
        return std::shared_ptr<Value>(data, &get<Value &>());
      } else {
        if (!data) {
          throw UndefinedAnyException();
        }
        return visitor_cast<T>(*data);
      }
    }

    /**
     * Casts the internal data to `T *` using `visitor_cast`.
     * `nullptr` will be returned if the cast is unsuccessful.
     */
    template <class T> T *tryGet() const {
      if (!data) {
        return nullptr;
      }
      return visitor_cast<T *>(data.get());
    }

    /**
     * Returns a shared pointer containing the result of `Visitor.tryGet<T>()`.
     * Will return an empty `shared_ptr` if unsuccessful.
     */
    template <class T> std::shared_ptr<T> getShared() const {
      if (auto ptr = tryGet<T>()) {
        return std::shared_ptr<T>(data, ptr);
      } else {
        return std::shared_ptr<T>();
      }
    }

    /**
     * `true`, when contains value, `false` otherwise
     */
    operator bool() const { return bool(data); }

    /**
     * resets the value
     */
    void reset() { data.reset(); }

    /**
     * the type of the stored value
     */
    TypeID type() const {
      if (!data) {
        return revisited::getTypeID<void>();
      }
      return data->visitableType();
    }

    /**
     * Accept visitor
     */
    void accept(VisitorBase &visitor) {
      if (!data) {
        throw UndefinedAnyException();
      }
      data->accept(visitor);
    }

    void accept(VisitorBase &visitor) const {
      if (!data) {
        throw UndefinedAnyException();
      }
      std::as_const(*data).accept(visitor);
    }

    bool accept(RecursiveVisitorBase &visitor) {
      if (!data) {
        throw UndefinedAnyException();
      }
      return data->accept(visitor);
    }

    bool accept(RecursiveVisitorBase &visitor) const {
      if (!data) {
        throw UndefinedAnyException();
      }
      return std::as_const(*data).accept(visitor);
    }

    /**
     * creation methods
     */

    template <class T, class VisitableType = typename AnyVisitable<T>::type, typename... Args>
    static Any create(Args &&... args) {
      Any a;
      a.set<T, VisitableType>(std::forward<Args>(args)...);
      return a;
    }

    template <typename... Bases, typename... Args> static Any withBases(Args &&... args) {
      Any a;
      a.setWithBases<Bases...>(std::forward<Args>(args)...);
      return a;
    }
  };

  template <class T, typename... Args> Any makeAny(Args &&... args) {
    Any v;
    v.set<T>(std::forward<Args>(args)...);
    return v;
  }

  // legacy type
  using AnyReference = Any;

  /**
   * Defines the default internal type for Any<T>.
   * Specialize this class to support implicit conversions usertypes.
   */
  template <class T> struct AnyVisitable {
    using type = typename std::conditional<std::is_base_of<VisitableBase, T>::value, T,
                                           DataVisitable<T>>::type;
  };

}  // namespace revisited

/**
 * Numeric any conversions.
 */
#define REVISITED_DEFINE_SCALAR_TYPE(Type, Conversions)                                            \
  template <> struct revisited::AnyVisitable<Type> {                                               \
    using Types = typename revisited::TypeList<Type &, const Type &>::template Merge<Conversions>; \
    using ConstTypes = typename revisited::TypeList<const Type &>::template Merge<Conversions>;    \
    using type = revisited::DataVisitablePrototype<Type, Types, ConstTypes>;                       \
  }

#ifndef REVISITED_NUMERIC_TYPES
#  define REVISITED_NUMERIC_TYPES                                                                \
    ::revisited::TypeList<char, unsigned char, short int, unsigned short int, int, unsigned int, \
                          long int, unsigned long int, long long int, unsigned long long int,    \
                          float, double, long double>
#endif

REVISITED_DEFINE_SCALAR_TYPE(char, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(unsigned char, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(short int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(unsigned short int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(unsigned int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(long int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(unsigned long int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(long long int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(unsigned long long int, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(float, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(double, REVISITED_NUMERIC_TYPES);
REVISITED_DEFINE_SCALAR_TYPE(long double, REVISITED_NUMERIC_TYPES);

/**
 * Char arrays are captured as strings.
 */
template <size_t N> struct revisited::AnyVisitable<char[N]> {
  using type = revisited::AnyVisitable<std::string>::type;
};

template <size_t N> struct revisited::AnyVisitable<const char[N]> {
  using type = revisited::AnyVisitable<std::string>::type;
};

/**
 * Capture values as reference through `std::reference_wrapper`.
 */
template <class T> struct revisited::AnyVisitable<std::reference_wrapper<T>> {
  using type = revisited::DataVisitablePrototype<
      std::reference_wrapper<T>, typename AnyVisitable<T>::type::Types,
      typename AnyVisitable<T>::type::ConstTypes, typename AnyVisitable<T>::type::Type>;
};

template <class T> struct revisited::AnyVisitable<std::reference_wrapper<const T>> {
  using type = revisited::DataVisitablePrototype<
      std::reference_wrapper<const T>, typename AnyVisitable<T>::type::ConstTypes,
      typename AnyVisitable<T>::type::ConstTypes, const typename AnyVisitable<T>::type::Type>;
};

/**
 * Allow casts of `shared_ptr` to value references.
 * Note: the origin `shared_ptr` cannot be reconstructed from the value.
 * Instead new `shared_ptr`s will be created for every call to
 * `Any::get<std::shared_ptr<T>>()`.
 */
template <class T> struct revisited::AnyVisitable<std::shared_ptr<T>> {
  using type = revisited::DataVisitablePrototype<
      revisited::any_detail::CapturedSharedPtr<T>,
      typename TypeList<std::shared_ptr<T> &>::template Merge<
          typename AnyVisitable<T>::type::Types>,
      typename TypeList<const std::shared_ptr<T> &, std::shared_ptr<T>>::template Merge<
          typename AnyVisitable<T>::type::ConstTypes>,
      T>;
};

namespace revisited {

  template <typename... Args> struct GetAnyVisitableTypes {
    using Types
        = revisited::TypeList<>::Merge<typename revisited::AnyVisitable<Args>::type::Types...>;
    using ConstTypes
        = revisited::TypeList<>::Merge<typename revisited::AnyVisitable<Args>::type::ConstTypes...>;
  };

}  // namespace revisited

#define REVISITED_DECLARE_BASES(TYPE, ...)                                                       \
  template <> struct revisited::AnyVisitable<TYPE> {                                             \
    using Types                                                                                  \
        = revisited::TypeList<TYPE &>::Merge<typename GetAnyVisitableTypes<__VA_ARGS__>::Types>; \
    using ConstTypes = revisited::TypeList<const TYPE &>::Merge<                                 \
        typename GetAnyVisitableTypes<__VA_ARGS__>::ConstTypes>;                                 \
    using type = revisited::DataVisitablePrototype<TYPE, Types, ConstTypes>;                     \
  }
