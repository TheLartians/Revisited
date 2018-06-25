#pragma once

#include <lars/visitor.h>
#include <lars/index_tuple.h>
#include <lars/make_function.h>
#include <lars/mutator.h>
#include <lars/type_index.h>

#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <type_traits>
#include <iterator>
#include <assert.h>

namespace lars{

  struct AnyScalarBase:public lars::Visitable<AnyScalarBase>{
    
  };
  
  template<class T> struct AnyScalarData:public DerivedVisitable<AnyScalarData<T>,WithVisitableBaseClass<AnyScalarBase>>::Type{
    T data;
    template <typename ... Args> AnyScalarData(Args && ... args):data(std::forward<Args>(args)...){ }
  };
  
  class Any{
  private:
    std::shared_ptr<AnyScalarBase> data;
  public:
    using BadAnyCast = lars::IncompatibleVisitorException;
        
    template <class T> T &get(){
      struct GetVisitor:public Visitor<AnyScalarData<T>>{
        T * result;
        void visit(AnyScalarData<T> &data){ result = &data.data; }
      } visitor;
      accept_visitor(visitor);
      return *visitor.result;
    }
    
    template <class T> const T &get()const{
      struct GetVisitor:public ConstVisitor<AnyScalarData<T>>{
        const T * result;
        void visit(const AnyScalarData<T> &data){ result = &data.data; }
      } visitor;
      accept_visitor(visitor);
      return *visitor.result;
    }
    
    template <class T> T *try_to_get(){
      struct GetVisitor:public Visitor<AnyScalarBase,AnyScalarData<T>>{
        T * result;
        void visit(AnyScalarBase &){ result = nullptr; }
        void visit(AnyScalarData<T> &data){ result = &data.data; }
      } visitor;
      accept_visitor(visitor);
      return visitor.result;
    }
    
    template <class T> const T * try_to_get()const{
      struct GetVisitor:public ConstVisitor<AnyScalarBase,AnyScalarData<T>>{
        const T * result;
        void visit(const AnyScalarBase &){ result = nullptr; }
        void visit(const AnyScalarData<T> &data){ result = &data.data; }
      } visitor;
      accept_visitor(visitor);
      return visitor.result;
    }

    template <class T = double> T get_numeric()const{
      struct GetVisitor:public ConstVisitor<AnyScalarData<float>,AnyScalarData<double>,AnyScalarData<unsigned>,AnyScalarData<int>,AnyScalarData<void*>>{
        T result;
        void visit(const AnyScalarData<float> &data){ result = data; }
        void visit(const AnyScalarData<double> &data){ result = data; }
        void visit(const AnyScalarData<int> &data){ result = data; }
        void visit(const AnyScalarData<unsigned> &data){ result = data; }
        void visit(const AnyScalarData<void*> &data){ result = data; }
      } visitor;
      accept_visitor(visitor);
      return visitor.result;
    }
    
    template <class T,typename ... Args> void set(Args && ... args){ data = std::make_unique<AnyScalarData<T>>(args...); }
    
    void accept_visitor(VisitorBase &visitor){ assert(data); data->accept(visitor); }
    void accept_visitor(ConstVisitorBase &visitor)const{ assert(data); data->accept(visitor); }
    
    operator bool()const{ return bool(data); }
  };
  
  template <class T,typename ... Args> Any make_any(Args && ... args){
    Any result;
    result.set<T>(std::forward<Args>(args)...);
    return result;
  }
  
  class AnyFunctionBase{
  public:
    virtual Any call_with_any_arguments(const std::vector<Any> &args)const = 0;
    virtual ~AnyFunctionBase(){}
    
    virtual unsigned argument_count()const = 0;
    virtual TypeIndex return_type()const = 0;
    virtual TypeIndex argument_type(unsigned)const = 0;
  };
  
  template <class R,typename ... Args> class AnyFunctionData:public AnyFunctionBase{
    template <class A,class B> struct SecondType{ using Type = B; };
    
  public:
    std::function<R(Args...)> data;
    
    AnyFunctionData(const std::function<R(Args...)> &f):data(f){}

    template <class U=R> typename std::enable_if<!std::is_void<U>::value,Any>::type call_with_any_arguments(const typename SecondType<Args,Any>::Type & ... args)const{
      return make_any<R>(data(args.template get< typename std::remove_const<typename std::remove_reference<Args>::type>::type >() ...));
    }
    
    template <class U=R> typename std::enable_if<std::is_void<U>::value,Any>::type call_with_any_arguments(const typename SecondType<Args,Any>::Type & ... args)const{
      data(args.template get< typename std::remove_const<typename std::remove_reference<Args>::type>::type >() ...);
      return Any();
    }
    
    template <size_t ... Indices> Any call_with_arguments_and_indices(const std::vector<Any> &args,StaticIndexTuple<Indices...> indices)const{
      return call_with_any_arguments(args[Indices] ...);
    }
    
    Any call_with_any_arguments(const std::vector<Any> &args)const override{
      if(args.size() != sizeof...(Args)) throw std::runtime_error("invalid argument count for function call");
      return call_with_arguments_and_indices(args,lars::IndexTupleRange<sizeof...(Args)>());
    }
    
    unsigned argument_count()const override{ return sizeof...(Args); }
    
    TypeIndex return_type()const override{ return get_type_index<R>(); }
    
    TypeIndex argument_type(unsigned i)const override{
      std::array<TypeIndex,sizeof...(Args)> types = {{ get_type_index<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()... }};
      return types[i];
    }
    
    virtual ~AnyFunctionData(){}
    
  };
  
  class AnyFunction{
  private:
    std::shared_ptr<AnyFunctionBase> data;
    template <class R,typename ... Args> void _set(const std::function<R(Args...)> &f){ data = std::make_unique<AnyFunctionData<R,Args...>>(f); }
  public:
    
    AnyFunction(){}
    template <class T> AnyFunction(T && f){ set(f); }
    template <class F> void set(F && f){ _set(make_function(f)); }
    
    Any call(const std::vector<Any> &args)const{ assert(data); return data->call_with_any_arguments(args); }

    template <typename ... Args> Any operator()(Args && ... args)const{
      assert(data);
      std::array<Any, sizeof...(Args)> tmp = {{make_any< typename std::remove_const<typename std::remove_reference<Args>::type>::type >(std::forward<Args>(args)) ...}};
      std::vector<Any> args_vector(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
      return call(args_vector);
    }
    
    unsigned argument_count()const{ return data->argument_count(); }
    TypeIndex return_type()const{ return data->return_type(); }
    TypeIndex argument_type(unsigned i)const{ return data->argument_type(i); }

    virtual ~AnyFunction(){}
  };
  
  template <class R,typename ... Args> AnyFunction make_any_function(const std::function<R(Args...)> &f){
    return AnyFunction(f);
  }
  
}
