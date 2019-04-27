#pragma once

#include <lars/any.h>
#include <lars/make_function.h>
#include <lars/dummy.h>

#include <functional>
#include <tuple>
#include <memory>
#include <exception>
#include <vector>

namespace lars {
  
  /**
   * Is raised when a undefined any function is called
   */
  struct AnyFunctionUndefinedCallException:public std::exception{
    const char * what() const noexcept override {
      return "called undefined AnyFunction";
    }
  };
  
  /**
   * Is raised when a any function is called with the wrong number of arguments
   */
  struct AnyFunctionInvalidArgumentCountException:public std::exception{
    const char * what() const noexcept override {
      return "called AnyFunction with wrong number of arguments";
    }
  };

  
  class AnyArguments:public std::vector<Any>{ using std::vector<Any>::vector; };
  
  struct SpecificAnyFunctionBase {
    virtual Any call(AnyArguments & args) const = 0;
    virtual ~SpecificAnyFunctionBase(){}
  };
  
  template <typename ... Args> class SpecificAnyFunction;
  
  template <class R, typename ... Args> class SpecificAnyFunction<R, Args...>: public SpecificAnyFunctionBase {
  private:
    std::function<R(Args...)> callback;
  
    template <size_t ... Idx> Any callWithArgumentIndices(AnyArguments & args, std::index_sequence<Idx...>) const {
      auto arguments = std::make_tuple(args[Idx].get<Args>()...);
      if constexpr (std::is_same<void, R>::value) {
        std::apply(callback, arguments);
        return Any();
      } else {
        return std::apply(callback, arguments);
      }
    }

  public:
    
    SpecificAnyFunction(std::function<R(Args...)> _callback):callback(_callback){}
    
    Any call(AnyArguments & args) const override {
      if (args.size() != sizeof...(Args)){ throw AnyFunctionInvalidArgumentCountException(); }
      using Indices = std::make_index_sequence<sizeof...(Args)>;
      return callWithArgumentIndices(args, Indices());
    }
    
  };
  
  template <class R> class SpecificAnyFunction<R, AnyArguments &>: public SpecificAnyFunctionBase {
  private:
    std::function<R(AnyArguments &)> callback;
  public:
    SpecificAnyFunction(std::function<R(AnyArguments &)> _callback):callback(_callback){}
    
    Any call(AnyArguments & args) const override {
      return callback(args);
    }

  };
  
  /**
   * Holds a functions of Any type.
   */
  class AnyFunction{
  private:
    std::shared_ptr<SpecificAnyFunctionBase> specific;
    
    template <class R,typename ... Args> void _set(const std::function<R(Args...)> &f){
      specific = std::make_shared<SpecificAnyFunction<R,Args...>>(f);
    }

  public:
    
    AnyFunction() = default;
    AnyFunction(const AnyFunction &) = default;
    AnyFunction(AnyFunction &&) = default;
    AnyFunction &operator=(const AnyFunction &) = default;
    AnyFunction &operator=(AnyFunction &&) = default;

    template <typename F> AnyFunction(const F &f) { set(f); }
    
    template <typename F> AnyFunction & operator=(const F & f){
      set(f);
      return *this;
    }
   
    template <typename F> void set(const F &f){
      _set(make_function(f));
    }
    
    Any call(AnyArguments & args) const {
      if (!specific) { throw AnyFunctionUndefinedCallException(); }
      return specific->call(args);
    }
    
    template <typename ... Args> Any operator()(Args && ... args) const {
      AnyArguments arguments;
      arguments.reserve(sizeof...(Args));
      dummy_function([&](){ arguments.emplace_back(args); return 0; }() ...);
      return call(arguments);
    }
    
    operator bool()const{
      return bool(specific);
    }
    
  };
  
}
