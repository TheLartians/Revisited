#pragma once

#include <revisited/any.h>
#include <revisited/make_function.h>

#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <vector>

namespace revisited {

  /**
   * Is raised when a undefined any function is called
   */
  struct UndefinedAnyFunctionException : public std::exception {
    const char *what() const noexcept override { return "use of undefined AnyFunction"; }
  };

  /**
   * Is raised when a any function is called with the wrong number of arguments
   */
  struct AnyFunctionInvalidArgumentCountException : public std::exception {
    const char *what() const noexcept override {
      return "called AnyFunction with wrong number of arguments";
    }
  };

  class AnyArguments : public std::vector<AnyReference> {
    using std::vector<AnyReference>::vector;
  };

  struct SpecificAnyFunctionBase {
    virtual Any call(const AnyArguments &args) const = 0;
    virtual TypeID returnType() const = 0;
    virtual TypeID argumentType(size_t) const = 0;
    virtual size_t argumentCount() const = 0;
    virtual bool isVariadic() const = 0;
    virtual ~SpecificAnyFunctionBase() {}
  };

  template <typename... Args> class SpecificAnyFunction;

  template <class R, typename... Args> class SpecificAnyFunction<R, Args...>
      : public SpecificAnyFunctionBase {
  private:
    std::function<R(Args...)> callback;

    template <size_t... Idx>
    Any callWithArgumentIndices(const AnyArguments &args, std::index_sequence<Idx...>) const {
      if constexpr (std::is_same<void, R>::value) {
        callback(args[Idx].get<Args>()...);
        return Any();
      } else {
        return callback(args[Idx].get<Args>()...);
      }
    }

  public:
    SpecificAnyFunction(std::function<R(Args...)> _callback) : callback(_callback) {}

    Any call(const AnyArguments &args) const override {
      if (args.size() != sizeof...(Args)) {
        throw AnyFunctionInvalidArgumentCountException();
      }
      using Indices = std::make_index_sequence<sizeof...(Args)>;
      return callWithArgumentIndices(args, Indices());
    }

    TypeID returnType() const override { return getTypeID<R>(); }

    size_t argumentCount() const override { return sizeof...(Args); }

    TypeID argumentType(size_t i) const override {
      if (i >= sizeof...(Args)) {
        return getTypeID<void>();
      } else {
        std::array<TypeID, sizeof...(Args)> argumentTypes{
            getTypeID<typename std::decay<Args>::type>()...};
        return argumentTypes[i];
      }
    }

    bool isVariadic() const override { return false; }
  };

  template <class R> class SpecificAnyFunction<R, const AnyArguments &>
      : public SpecificAnyFunctionBase {
  private:
    std::function<R(const AnyArguments &)> callback;

  public:
    SpecificAnyFunction(std::function<R(const AnyArguments &)> _callback) : callback(_callback) {}

    Any call(const AnyArguments &args) const override {
      if constexpr (std::is_same<void, R>::value) {
        callback(args);
        return Any();
      } else {
        return callback(args);
      }
    }

    TypeID returnType() const override { return getTypeID<R>(); }

    size_t argumentCount() const override { return 0; }

    TypeID argumentType(size_t) const override { return getTypeID<Any>(); }

    bool isVariadic() const override { return true; }
  };

  /**
   * Holds a functions of Any type.
   */
  class AnyFunction {
  private:
    std::shared_ptr<SpecificAnyFunctionBase> specific;

    template <class R, typename... Args> void _set(const std::function<R(Args...)> &f) {
      specific = std::make_shared<SpecificAnyFunction<R, Args...>>(f);
    }

  public:
    AnyFunction() = default;
    AnyFunction(const AnyFunction &) = default;
    AnyFunction(AnyFunction &&) = default;
    AnyFunction &operator=(const AnyFunction &) = default;
    AnyFunction &operator=(AnyFunction &&) = default;

    template <typename F,
              typename = typename std::enable_if<!std::is_convertible<F, AnyFunction>::value>::type>
    AnyFunction(const F &f) {
      set(f);
    }

    template <typename F,
              typename = typename std::enable_if<!std::is_convertible<F, AnyFunction>::value>::type>
    AnyFunction &operator=(const F &f) {
      set(f);
      return *this;
    }

    template <typename F> void set(const F &f) {
      static_assert(!std::is_convertible<F, AnyFunction>::value);
      _set(make_function(f));
    }

    Any call(const AnyArguments &args) const {
      if (!specific) {
        throw UndefinedAnyFunctionException();
      }
      return specific->call(args);
    }

    template <typename... Args> Any operator()(Args &&... args) const {
      AnyArguments arguments{{[&]() {
        using ArgType = typename any_detail::remove_cvref<Args>::type;
        if constexpr (std::is_base_of<Any, ArgType>::value) {
          return AnyReference(args);
        } else if constexpr (std::is_same<typename AnyVisitable<ArgType>::type::Type,
                                          ArgType>::value) {
          return AnyReference(
              std::reference_wrapper<typename std::remove_reference<Args>::type>(args));
        } else {
          return AnyReference(args);
        }
      }()}...};
      return call(arguments);
    }

    explicit operator bool() const { return bool(specific); }

    TypeID returnType() const {
      if (!specific) {
        throw UndefinedAnyFunctionException();
      }
      return specific->returnType();
    }

    TypeID argumentType(size_t i) const {
      if (!specific) {
        throw UndefinedAnyFunctionException();
      }
      return specific->argumentType(i);
    }

    size_t argumentCount() const {
      if (!specific) {
        throw UndefinedAnyFunctionException();
      }
      return specific->argumentCount();
    }

    bool isVariadic() const { return specific->isVariadic(); }
  };

}  // namespace revisited
