#pragma once

#include <lars/any.h>
#include <lars/make_function.h>

#include <functional>
#include <memory>
#include <exception>
#include <array>
#include <vector>

namespace lars {
  
  /**
   * Is raised when a undefined any function is called
   */
  struct UndefinedAnyFunctionException:public std::exception{
    const char * what() const noexcept override {
      return "use of undefined AnyFunction";
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

  class AnyArguments:public std::vector<AnyReference>{ 
    using std::vector<AnyReference>::vector; 
  };
  
  struct SpecificAnyFunctionBase {
    virtual AnyReference call(const AnyArguments & args) const = 0;
    virtual NamedTypeIndex returnType()const = 0;
    virtual NamedTypeIndex argumentType(size_t)const = 0;
    virtual size_t argumentCount()const = 0;
    virtual ~SpecificAnyFunctionBase(){}
  };
  
  template <typename ... Args> class SpecificAnyFunction;
  
  template <class R, typename ... Args> class SpecificAnyFunction<R, Args...>: public SpecificAnyFunctionBase {
  private:
    std::function<R(Args...)> callback;
  
    template <size_t ... Idx> AnyReference callWithArgumentIndices(const AnyArguments & args, std::index_sequence<Idx...>) const {
      if constexpr (std::is_same<void, R>::value) {
        callback(args[Idx].get<Args>()...);
        return Any();
      } else {
        return callback(args[Idx].get<Args>()...);
      }
    }

  public:
    
    SpecificAnyFunction(std::function<R(Args...)> _callback):callback(_callback){}
    
    AnyReference call(const AnyArguments & args) const override {
      if (args.size() != sizeof...(Args)){ throw AnyFunctionInvalidArgumentCountException(); }
      using Indices = std::make_index_sequence<sizeof...(Args)>;
      return callWithArgumentIndices(args, Indices());
    }

    NamedTypeIndex returnType()const override{
      return getNamedTypeIndex<R>();
    }
    
    size_t argumentCount()const override{
      return sizeof...(Args);
    }
    
    NamedTypeIndex argumentType(size_t i)const override{
      if (i >= sizeof...(Args)) {
        return getNamedTypeIndex<void>();
      } else {
        std::array<NamedTypeIndex,sizeof...(Args)> argumentTypes{
          getNamedTypeIndex<typename std::remove_reference<typename std::remove_const<Args>::type>::type>()...
        };
        return argumentTypes[i];
      }
    }
    
  };
  
  template <class R> class SpecificAnyFunction<R, const AnyArguments &>: public SpecificAnyFunctionBase {
  private:
    std::function<R(const AnyArguments &)> callback;
  public:
    SpecificAnyFunction(std::function<R(const AnyArguments &)> _callback):callback(_callback){}
    
    AnyReference call(const AnyArguments & args) const override {
      return callback(args);
    }
    
    NamedTypeIndex returnType()const override{
      return getNamedTypeIndex<R>();
    }
    
    size_t argumentCount()const override{
      return 1;
    }
    
    NamedTypeIndex argumentType(size_t i)const override{
      if (i >= 1) {
        return getNamedTypeIndex<void>();
      } else {
        return getNamedTypeIndex<AnyArguments>();
      }
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
    
    AnyReference call(const AnyArguments & args) const {
      if (!specific) { throw UndefinedAnyFunctionException(); }
      return specific->call(args);
    }
    
    template <typename ... Args> AnyReference operator()(Args && ... args) const {
      AnyArguments arguments{{[&](){
        using ArgType = typename std::remove_reference<Args>::type;
        if constexpr (std::is_same<ArgType, Any>::value) {
          return AnyReference(args);
        } else {
          return AnyReference(std::reference_wrapper<ArgType>(args));
        }
      }()} ...};
      return call(arguments);
    }
    
    operator bool()const{
      return bool(specific);
    }
    
    NamedTypeIndex returnType()const{
      if (!specific) { throw UndefinedAnyFunctionException(); }
      return specific->returnType();
    }
    
    NamedTypeIndex argumentType(size_t i)const{
      if (!specific) { throw UndefinedAnyFunctionException(); }
      return specific->argumentType(i);
    }
    
    size_t argumentCount()const{
      if (!specific) { throw UndefinedAnyFunctionException(); }
      return specific->argumentCount();
    }

    
  };
  
}
