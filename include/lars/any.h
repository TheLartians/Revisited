#pragma once

#include <lars/visitor.h>
#include <lars/index_tuple.h>
#include <lars/make_function.h>
#include <lars/mutator.h>
#include <lars/type_index.h>
#include <lars/visitable_type_index.h>

#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <type_traits>
#include <iterator>
#include <string>
#include <assert.h>

namespace lars{

  template<class T> struct VisitableScalar:public lars::Visitable<VisitableScalar<T>>{
    T data;
    template <typename ... Args> VisitableScalar(Args && ... args):data(std::forward<Args>(args)...){ }
    operator T &(){ return data; }
  };
  
  template <class T> using VisitableType = typename std::conditional<is_visitable<T>::value, T, VisitableScalar<T>>::type;
  
  class Any{
  private:
    std::shared_ptr<VisitableBase> data;
    TypeIndex _type = lars::get_type_index<void>();

  public:
    class BadAnyCast:public std::runtime_error{ using std::runtime_error::runtime_error; };
    
    TypeIndex type()const{ return _type; }
    
    template <class T> T &get(){
      struct GetVisitor:public RecursiveVisitor<VisitableScalar<T>,T>{
        T * result;
        bool visit(T &obj){ result = &obj; return false; }
        bool visit(VisitableScalar<T> &data){ result = &(T&)data; return false; }
      } visitor;
      accept_visitor(visitor);
      if(visitor.result) return *visitor.result;
      else throw BadAnyCast("cannot convert " + std::string(type().name().begin(),type().name().end()) + " to " + get_type_name<T>());
    }
    
    template <class T> const T &get()const{
      struct GetVisitor:public RecursiveConstVisitor<VisitableScalar<T>,T>{
        const T * result;
        bool visit(const T &obj){ result = &obj; return false; }
        bool visit(const VisitableScalar<T> &data){ result = &(T&)data; return false; }
      } visitor;
      accept_visitor(visitor);
      if(visitor.result) return *visitor.result;
      else throw BadAnyCast("cannot convert " + std::string(type().name().begin(),type().name().end()) + " to " + get_type_name<T>());
    }
    
    template <class T = double> T get_numeric()const{
      struct GetVisitor:public ConstVisitor<VisitableType<float>,VisitableType<double>,VisitableType<unsigned>,VisitableType<int>,VisitableType<bool>,VisitableType<char>>{
        T result;
        void visit(const VisitableType<bool> &data){ result = data.data; }
        void visit(const VisitableType<char> &data){ result = data.data; }
        void visit(const VisitableType<float> &data){ result = data.data; }
        void visit(const VisitableType<double> &data){ result = data.data; }
        void visit(const VisitableType<int> &data){ result = data.data; }
        void visit(const VisitableType<unsigned> &data){ result = data.data; }
      } visitor;
      try{
        accept_visitor(visitor);
        return visitor.result;
      }
      catch(lars::IncompatibleVisitorException){
        throw BadAnyCast("cannot convert " + std::string(type().name().begin(),type().name().end()) + " to " + get_type_name<T>());
      }
    }
    
    template <class T> typename std::enable_if<std::is_arithmetic<T>::value,T>::type smart_get(){
      return get_numeric<T>();
    }
    
    template <class T> typename std::enable_if<!std::is_arithmetic<T>::value && is_visitable<T>::value,T &>::type smart_get(){
      
      return get<T>();
    }
    
    template <class T> typename std::enable_if<!std::is_arithmetic<T>::value && !is_visitable<T>::value,T &>::type smart_get(){
      return get<T>();
    }
    
    template <class T,typename ... Args> typename std::enable_if<!std::is_array<T>::value,void>::type set(Args && ... args){
      data = std::make_unique<VisitableType<T>>(std::forward<Args>(args)...);
      _type = lars::get_type_index<T>();
    }
    
    template <class T,typename ... Args> typename std::enable_if<std::is_array<T>::value,void>::type set(Args && ... args){
      data = std::make_unique<VisitableType<std::basic_string<typename std::remove_extent<T>::type>>>(std::forward<Args>(args)...);
      _type = lars::get_type_index<T>();
    }

    template <class Visitor> void accept_visitor(Visitor &visitor){ assert(data); data->accept(visitor); }
    template <class ConstVisitor> void accept_visitor(ConstVisitor &visitor)const{ assert(data); data->accept(visitor); }
    
    operator bool()const{ return bool(data); }
  };
  
  template <class T,typename ... Args> typename std::enable_if<!std::is_same<Any, T>::value,Any>::type make_any(Args && ... args){
    Any result;
    result.set<T>(std::forward<Args>(args)...);
    return result;
  }
  
  template <class T,typename ... Args> typename std::enable_if<std::is_same<Any, T>::value,Any>::type make_any(Args && ... args){
    return Any(args...);
  }
  
  class AnyArguments:public std::vector<Any>{ using std::vector<Any>::vector; };
  
  class AnyFunctionBase{
  public:
    virtual ~AnyFunctionBase(){}
    
    virtual Any call_with_any_arguments(AnyArguments &args)const = 0;
    virtual int argument_count()const = 0;
    virtual TypeIndex return_type()const = 0;
    virtual TypeIndex argument_type(unsigned)const = 0;
  };
  
  template <class R,typename ... Args> class AnyFunctionData;
  
  template <class R,typename ... Args> class AnyFunctionData:public AnyFunctionBase{
    template <class A,class B> struct SecondType{ using Type = B; };
    
  public:
    std::function<R(Args...)> data;
    
    AnyFunctionData(const std::function<R(Args...)> &f):data(f){}

    template <class U=R> typename std::enable_if<!std::is_void<U>::value,Any>::type call_with_any_arguments(typename SecondType<Args,Any>::Type & ... args)const{
      return make_any<R>(data(args.template smart_get< typename std::remove_const<typename std::remove_reference<Args>::type>::type >() ...));
    }
    
    template <class U=R> typename std::enable_if<std::is_void<U>::value,Any>::type call_with_any_arguments(typename SecondType<Args,Any>::Type & ... args)const{
      data(args.template smart_get< typename std::remove_const<typename std::remove_reference<Args>::type>::type >() ...);
      return Any();
    }
    
    template <size_t ... Indices> Any call_with_arguments_and_indices(AnyArguments &args,StaticIndexTuple<Indices...> indices)const{
      return call_with_any_arguments(args[Indices] ...);
    }
    
    Any call_with_any_arguments(AnyArguments &args)const override{
      if(args.size() != sizeof...(Args)) throw std::runtime_error("invalid argument count for function call");
      return call_with_arguments_and_indices(args,lars::IndexTupleRange<sizeof...(Args)>());
    }
    
    int argument_count()const override{ return sizeof...(Args); }
    
    TypeIndex return_type()const override{ return get_type_index<R>(); }
    
    template <class D = TypeIndex> typename std::enable_if<(sizeof...(Args) > 0), D>::type _argument_type(unsigned i)const{
      constexpr std::array<TypeIndex,sizeof...(Args)> types = {{ get_type_index<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()... }};
      return types[i];
    }

    template <class D = TypeIndex> typename std::enable_if<sizeof...(Args) == 0, D>::type _argument_type(unsigned i)const{
      throw std::runtime_error("invalid argument");
    }

    TypeIndex argument_type(unsigned i)const override{
      return _argument_type<>(i);
    }
    
  };
  
  template <class R> class AnyFunctionData<R,AnyArguments &>:public AnyFunctionBase{
  public:
    std::function<R(AnyArguments &)> data;
    AnyFunctionData(const std::function<R(AnyArguments &)> &f):data(f){}
    template <class T=R> typename std::enable_if<!std::is_void<T>::value,Any>::type _call_with_any_arguments(AnyArguments &args)const{ return make_any<R>(data(args)); }
    template <class T=R> typename std::enable_if<std::is_void<T>::value,Any>::type _call_with_any_arguments(AnyArguments &args)const{ data(args); return Any(); }
    Any call_with_any_arguments(AnyArguments &args)const override{ return _call_with_any_arguments(args); }
    int argument_count()const override{ return -1; }
    TypeIndex return_type()const override{ return get_type_index<R>(); }
    TypeIndex argument_type(unsigned)const override{ return get_type_index<Any>(); }
  };
  
  class AnyFunction{
  private:
    std::shared_ptr<AnyFunctionBase> data;
    template <class R,typename ... Args> void _set(const std::function<R(Args...)> &f){ data = std::make_unique<AnyFunctionData<R,Args...>>(f); }
  public:
    
    AnyFunction(){}
    template <class T> AnyFunction(const T & f){ set(f); }
    template <class F> void set(const F & f){ _set(make_function(f)); }

    // template <class T> AnyFunction(T && f){ set(f); }
    //template <class F> void set(F && f){ _set(make_function(f)); }
    
    Any call(AnyArguments &args)const{ assert(data); return data->call_with_any_arguments(args); }

    template <typename ... Args> Any operator()(Args && ... args)const{
      std::array<Any, sizeof...(Args)> tmp = {{make_any< typename std::remove_const<typename std::remove_reference<Args>::type>::type >(std::forward<Args>(args)) ...}};
      AnyArguments args_vector(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
      return call(args_vector);
    }
    
    int argument_count()const{ return data->argument_count(); }
    TypeIndex return_type()const{ return data->return_type(); }
    TypeIndex argument_type(unsigned i)const{ return data->argument_type(i); }
    operator bool()const{ return data.operator bool(); }
    
    virtual ~AnyFunction(){}
  };
  
  template <class R,typename ... Args> AnyFunction make_any_function(const std::function<R(Args...)> &f){
    return AnyFunction(f);
  }
  
}
