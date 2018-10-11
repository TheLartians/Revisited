
#pragma once

//#define VISITOR_DEBUG

#include <lars/dummy.h>
#include <lars/type_index.h>

#include <exception>
#include <type_traits>

#ifdef VISITOR_DEBUG
#include <iostream>
#define VISITOR_LOG(X) { std::cout << "visitor: " << this << ": " << X << std::endl; }
#else
#define VISITOR_LOG(X)
#endif

namespace lars{
  
#pragma mark -
#pragma mark Helper
  namespace visitor_helper{
  
    template <typename ... Args> class TypeList{};

    template <class T,int ORDER> struct OrderedType{ using Type = T; const static int value = ORDER; };
    template <typename ... Args> class OrderedTypeSet{};
    
    template <int ... Args> struct HighestOrder;
    template <int first,int ... Rest> struct HighestOrder<first,Rest...>{ const static int value = HighestOrder<Rest...>::value; };
    template <int last> struct HighestOrder<last>{ const static int value = last; };
    template <> struct HighestOrder<>{ const static int value = 0; };
    
    template <typename ... Args> struct HighestOrderInOrderedTypeSet;
    template <typename ... Args> struct HighestOrderInOrderedTypeSet<OrderedTypeSet<Args...>>{ const static int value = HighestOrder<Args::value...>::value; };
    
    template <int ... Args> struct LowestOrder;
    template <int first,int ... Rest> struct LowestOrder<first,Rest...>{ const static int value = first; };
    template <> struct LowestOrder<>{ const static int value = 0; };
    
    template <typename ... Args> struct LowestOrderInOrderedTypeSet;
    template <typename ... Args> struct LowestOrderInOrderedTypeSet<OrderedTypeSet<Args...>>{ const static int value = LowestOrder<Args::value...>::value; };
    
    
    template <typename ... Args> class Before{};
    
    template <typename ... Args> struct OrderedTypeSetContains;
    template <class First,int FORDER,typename ... Args,class T> struct OrderedTypeSetContains<OrderedTypeSet<OrderedType<First, FORDER>,Args...>,T>:public OrderedTypeSetContains<OrderedTypeSet<Args...>,T>{ };
    template <typename ... Args,class T,int ORDER> struct OrderedTypeSetContains<OrderedTypeSet<OrderedType<T,ORDER>,Args...>,T>:public std::true_type{ };
    template <class T> struct OrderedTypeSetContains<OrderedTypeSet<>,T>:public std::false_type{ };
    
    template <typename ... Args> class AddToOrderedTypeSet;
    template <typename ... Args,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>>{
    public:
      using Type = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<>>::Type;
    };
    
    template <typename ... Args,typename ... BArgs,class FT,int FORDER,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<OrderedType<FT,FORDER>,Args...>,OrderedType<T,ORDER>,Before<BArgs...>>{
      static const bool insert_before = ORDER < FORDER;
      using Contained = OrderedTypeSetContains<OrderedTypeSet<Args...>,T>;
      
      using IfContained = OrderedTypeSet<BArgs...,OrderedType<FT,FORDER>,Args...> ;
      using IfNotContained = OrderedTypeSet<BArgs...,OrderedType<T,ORDER>,OrderedType<FT,FORDER>,Args...> ;
      
      using If = typename std::conditional<Contained::value,IfContained,IfNotContained>::type;
      using Else = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<BArgs...,OrderedType<FT,FORDER>>>::Type;
    public:
      using Type = typename std::conditional<insert_before,If,Else>::type;
    };
    
    template <typename ... Args,typename ... BArgs,int FORDER,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<OrderedType<T,FORDER>,Args...>,OrderedType<T,ORDER>,Before<BArgs...>>{
      static const bool insert_before = ORDER < FORDER;
      using If = OrderedTypeSet<BArgs...,OrderedType<T,ORDER>,Args...> ;
      using Else = typename AddToOrderedTypeSet<OrderedTypeSet<Args...>,OrderedType<T,ORDER>,Before<BArgs...>>::Type;
    public:
      using Type = typename std::conditional<insert_before,If,Else>::type;
    };
    
    template <typename ... BArgs,class T,int ORDER> class AddToOrderedTypeSet<OrderedTypeSet<>,OrderedType<T,ORDER>,Before<BArgs...>>{
    public:
      using Type = OrderedTypeSet<BArgs...,OrderedType<T, ORDER>>;
    };
    
    template <class A,class B> struct JoinOrderedTypeSets;
    template <typename ... ArgsA,class ArgsB> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<ArgsB>>{
      using Type = typename AddToOrderedTypeSet<OrderedTypeSet<ArgsA...>,ArgsB>::Type;
    };
    
    template <typename ... ArgsA,class First,typename ... ArgsB> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<First,ArgsB...>>{
      using Type = typename JoinOrderedTypeSets< typename AddToOrderedTypeSet<OrderedTypeSet<ArgsA...>,First>::Type, OrderedTypeSet<ArgsB...> >::Type;
    };
    
    template <typename ... ArgsA> struct JoinOrderedTypeSets<OrderedTypeSet<ArgsA...>,OrderedTypeSet<>>{
      using Type = OrderedTypeSet<ArgsA...>;
    };
    
    template<typename T> struct to_void { typedef void type; };
    
    template <typename ... Args> struct ExtractBaseTypes;
    
    template <typename T, typename = void> struct AllBaseTypesOfSingleType{ using Type = OrderedTypeSet<>; };
    template <typename T> struct AllBaseTypesOfSingleType <T, typename to_void<typename T::VisitableBaseTypes>::type>{ using Type = typename ExtractBaseTypes<typename T::VisitableBaseTypes>::Type; };
    
    template <typename ... Args> struct AllBaseTypes;
    template <class First,typename ... Args> struct AllBaseTypes<First,Args...>{
      using Type = typename JoinOrderedTypeSets<typename AllBaseTypes<First>::Type, typename AllBaseTypes<Args...>::Type>::Type;
    };
    template <class T> struct AllBaseTypes<T>{ using Type = typename AllBaseTypesOfSingleType<T>::Type; };
    
    template <> struct ExtractBaseTypes<OrderedTypeSet<>>{ using Type = OrderedTypeSet<>; };
    template <class First,typename ... Args> struct ExtractBaseTypes<OrderedTypeSet<First,Args...>>{
      using TypeOfFirst = typename AddToOrderedTypeSet< typename AllBaseTypes<First>::Type, First >::Type;
      using Type = typename JoinOrderedTypeSets<TypeOfFirst,typename ExtractBaseTypes<OrderedTypeSet<Args...>>::Type>::Type;
    };
    
#ifdef VISITOR_DEBUG
    template <class Stream,class T,int ORDER> Stream &operator<<(Stream &stream,OrderedType<T,ORDER>){
      stream << '(' << lars::get_type_name<T>() << ',' << ORDER << ')';
      return stream;
    }
    
    template <class Stream,typename ... Args> Stream &operator<<(Stream &stream,OrderedTypeSet<Args...>){
      stream << '{';
      lars::dummy_function(operator<<(stream,Args())...);
      stream << '}';
      return stream;
    }
#endif
   
    template <typename ... Args> struct GetFirst;
    template <class First, class... Rest> struct GetFirst<First,Rest...>{ using type = First; };
    template <class First> struct GetFirst<First>{ using type = First; };
    template <> struct GetFirst<>{ using type = void; };

  }
  
#pragma mark -
#pragma mark Predefinitions
  
  class VisitorBase;
  class ConstVisitorBase;
  class RecursiveVisitorBase;
  class RecursiveConstVisitorBase;
  
  template <typename ... Args> class WithBaseClass{};
  template <typename ... Args> class WithVirtualBaseClass{};
  template <typename ... Args> class WithVisitableBaseClass{};
  template <typename ... Args> class WithStaticVisitor{};
  
  class VisitableBase;
  
  template <typename ... Args> class Visitable;
  template <typename ... Args> class StaticVisitable;
  template <typename ... Args> class VisitableAndStaticVisitable;
  
#pragma mark -
#pragma mark Visitor
  
  struct IncompatibleVisitorException:public std::exception{
    const char* what() const throw ()override{ return "IncompatibleVisitorException"; }
  };

  template <typename ... Args> class VisitorPrototype;
  
  template <class Base,typename Visiting,typename ... VisitingRest,class Accepted,typename ... AcceptedRest,class Return> class VisitorPrototype<Base,visitor_helper::TypeList<Visiting,VisitingRest...>,visitor_helper::TypeList<Accepted,AcceptedRest...>,Return>:public virtual Base,public VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>,public VisitorPrototype<Base,visitor_helper::TypeList<VisitingRest...>,visitor_helper::TypeList<AcceptedRest...>,Return>{
  protected:
    
    void * cast_to_visitor_for(TypeIndex type_id){
      auto result = VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>::cast_to_visitor_for(type_id);
      if(result) return result;
      return VisitorPrototype<Base,visitor_helper::TypeList<VisitingRest...>,visitor_helper::TypeList<AcceptedRest...>,Return>::cast_to_visitor_for(type_id);
    }
    
  public:
    using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Visiting,VisitingRest...>,visitor_helper::TypeList<const Visiting &,const VisitingRest &...>,Return>;
    
    void * as_visitor_for(TypeIndex requested)override{
      return cast_to_visitor_for(requested);
    }
    
  };
  
  template <class Base,typename Visiting,class Accepted,class Return> class VisitorPrototype<Base,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<Accepted>,Return>:public virtual Base{
  protected:
    void * cast_to_visitor_for(TypeIndex requested){
      VISITOR_LOG("casting Visitor<" << get_type_name<Visiting>() << "> to visitor for: " << requested.name());
      if(get_type_index<Visiting>() == requested){ return reinterpret_cast<void*>(this); }
      return nullptr;
    }
    
  public:
    using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Visiting>,visitor_helper::TypeList<const Visiting &>,Return>;
    
    void * as_visitor_for(TypeIndex requested)override{
      return cast_to_visitor_for(requested);
    }
    
    virtual Return visit(Accepted) = 0;
  };
  
  template <typename ... Args> using Visitor = VisitorPrototype<VisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<Args &...>,void>;
  template <typename ... Args> using ConstVisitor = VisitorPrototype<ConstVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<const Args &...>,void>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<RecursiveVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<Args &...>,bool>;
  template <typename ... Args> using RecursiveConstVisitor = VisitorPrototype<RecursiveConstVisitorBase,visitor_helper::TypeList<Args...>,visitor_helper::TypeList<const Args &...>,bool>;
  
#pragma mark -
#pragma mark Base Classes
  
  template <template <typename ... Args> class Visitor,class Default> class VisitorBasePrototype{
  public:
    using TypeIndex = lars::TypeIndex;
    virtual void * as_visitor_for(TypeIndex) = 0;
    
    template <class T> Visitor<T> * as_visitor_for(){
      return reinterpret_cast<Visitor<T> *>(as_visitor_for(get_type_index<T>()));
    }
    
    virtual void visit_default(Default & visitable){
      throw IncompatibleVisitorException();
    }
    
    virtual ~VisitorBasePrototype(){}
  };
  
  class VisitorBase:public VisitorBasePrototype<Visitor,VisitableBase>{};
  class ConstVisitorBase:public VisitorBasePrototype<ConstVisitor,const VisitableBase>{};
  class RecursiveVisitorBase:public VisitorBasePrototype<RecursiveVisitor,VisitableBase>{};
  class RecursiveConstVisitorBase:public VisitorBasePrototype<RecursiveConstVisitor,const VisitableBase>{};

  class VisitableBase{
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(ConstVisitorBase &visitor)const = 0;
    virtual void accept(RecursiveVisitorBase &visitor) = 0;
    virtual void accept(RecursiveConstVisitorBase &visitor)const = 0;
    virtual ~VisitableBase(){}
  };
  
  
#pragma mark -
#pragma mark Visitable
  
  template <class T> class Visitable<T>:public virtual VisitableBase{
  public:
    
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};
    
    void accept(VisitorBase &visitor)override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
      }
      else{
        visitor.visit_default(static_cast<VisitableBase&>(*this));
      }
    }
    
    void accept(ConstVisitorBase &visitor)const override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<const T &>(*this));
      }
      else{
        visitor.visit_default(static_cast<const VisitableBase &>(*this));
      }
    }
    
    void accept(RecursiveVisitorBase &visitor)override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<T &>(*this));
      }
    }
    
    void accept(RecursiveConstVisitorBase &visitor)const override{
      if(auto casted = visitor.as_visitor_for<T>()){
        casted->visit(static_cast<const T &>(*this));
      }
    }
    
  };
  
  template <class ... Base,class ... VBases,class ... VisitableBases,class OrderedBases> class alignas(VisitableBase) alignas(Base...) alignas(VBases...) Visitable<WithBaseClass<Base...>,WithVirtualBaseClass<VBases...>,WithVisitableBaseClass<VisitableBases...>,OrderedBases>:public virtual VisitableBase,public Base...,public virtual VBases...{
  public:
    
    Visitable(){}
    template<typename ... Args,typename First = typename std::enable_if<sizeof...(Base) == 1 && sizeof...(Args) == sizeof...(Args),typename visitor_helper::GetFirst<Base...>::type>::type> Visitable(Args ... args):First(args...){}
    
  private:
    
    struct IncompatibleVisitorException:public lars::IncompatibleVisitorException{};

    template <class Current> void try_to_accept(VisitorBase &visitor){
      VISITOR_LOG("try to accept visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<Current &>(*this));
      }
      else{
        visitor.visit_default(static_cast<VisitableBase&>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(VisitorBase &visitor){
      VISITOR_LOG("try to accept visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<Current &>(*this));
        return;
      }
      VISITOR_LOG("Continue with: " << lars::get_type_name<Second>());
      try_to_accept<Second,Rest ...>(visitor);
    }
    
    template <class Current> void try_to_accept(ConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept const_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<const Current &>(*this));
      }
      else{
        visitor.visit_default(static_cast<const VisitableBase &>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(ConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept const_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        casted->visit(static_cast<const Current &>(*this));
        VISITOR_LOG("Success!");
        return;
      }
      VISITOR_LOG("Continue with: " << lars::get_type_name<Second>());
      try_to_accept<Second,Rest ...>(visitor);
    }
    
    template <class Current> void try_to_accept(RecursiveVisitorBase &visitor){
      VISITOR_LOG("try to accept recursive_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<Current &>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(RecursiveVisitorBase &visitor){
      VISITOR_LOG("try to accept recursive_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        if(!casted->visit(static_cast<Current &>(*this))) return;
      }
      VISITOR_LOG("Continue with: " << lars::get_type_name<Second>());
      try_to_accept<Second,Rest ...>(visitor);
    }

    template <class Current> void try_to_accept(RecursiveConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept recursive_const_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        casted->visit(static_cast<const Current &>(*this));
      }
    }
    
    template <class Current,class Second,typename ... Rest> void try_to_accept(RecursiveConstVisitorBase &visitor)const{
      VISITOR_LOG("try to accept recursive_const_visitor: " << lars::get_type_name<Current>());
      if(auto casted = visitor.as_visitor_for<Current>()){
        VISITOR_LOG("Success!");
        if(!casted->visit(static_cast<const Current &>(*this))) return;
      }
      VISITOR_LOG("Continue with: " << lars::get_type_name<Second>());
      try_to_accept<Second,Rest ...>(visitor);
    }

  public:
    
    using VisitableBaseTypes = OrderedBases;
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

    void accept(VisitorBase &visitor)override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(ConstVisitorBase &visitor)const override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(RecursiveVisitorBase &visitor)override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
    void accept(RecursiveConstVisitorBase &visitor)const override{
      try_to_accept<VisitableBases...>(visitor);
    }
    
#pragma clang diagnostic pop

  };
  
  template <typename ... Args> struct ToWithVisitableBaseClass;
  template <typename ... Args> struct ToWithVisitableBaseClass<visitor_helper::OrderedTypeSet<Args...>>{
    using Type = WithVisitableBaseClass<typename Args::Type ...>;
  };
  
#pragma mark -
#pragma mark Derived Visitable
  
#define LARS_REMOVE_VISITABLE_ACCEPT_METHODS() void accept(lars::VisitorBase&)override = 0; void accept(lars::ConstVisitorBase&)const override = 0; void accept(lars::RecursiveVisitorBase&)override = 0; void accept(lars::RecursiveConstVisitorBase&)const override = 0
  
#define LARS_DEFINE_FORWARDED_VISITABLE_ACCEPT_METHODS(BASE) \
  void accept(::lars::VisitorBase &visitor)override{ BASE::accept(visitor); }\
  void accept(::lars::ConstVisitorBase &visitor)const override{ BASE::accept(visitor); }\
  void accept(::lars::RecursiveVisitorBase &visitor)override{ BASE::accept(visitor); }\
  void accept(::lars::RecursiveConstVisitorBase &visitor)const override{ BASE::accept(visitor); }
  
  template <typename ... Args> class DerivedVisitable;
  
  template <typename ... Bases,typename ... VisitableBases> class DerivedVisitable<WithVirtualBaseClass<Bases...>,WithVisitableBaseClass<VisitableBases...>>{
    using AllBaseTypes = typename visitor_helper::AllBaseTypes<VisitableBases...>::Type;
    static const int current_order = visitor_helper::LowestOrderInOrderedTypeSet<AllBaseTypes>::value;
  public:
    using BaseTypeList = typename visitor_helper::JoinOrderedTypeSets<AllBaseTypes, visitor_helper::OrderedTypeSet<visitor_helper::OrderedType<VisitableBases,current_order>...> >::Type;
    using Type = Visitable<WithBaseClass<>,WithVirtualBaseClass<Bases...> , typename ToWithVisitableBaseClass<BaseTypeList>::Type, BaseTypeList>;
  };
  
  template <class T,typename ... Bases,typename ... VisitableBases> class DerivedVisitable<T,WithVirtualBaseClass<Bases...>,WithVisitableBaseClass<VisitableBases...>>{
    using AllBaseTypes = typename visitor_helper::AllBaseTypes<VisitableBases...>::Type;
    static const int current_order = visitor_helper::LowestOrderInOrderedTypeSet<AllBaseTypes>::value;
  public:
    using BaseTypeList = typename visitor_helper::JoinOrderedTypeSets<AllBaseTypes, visitor_helper::OrderedTypeSet<visitor_helper::OrderedType<T,current_order-1>,visitor_helper::OrderedType<VisitableBases,current_order>...> >::Type;
    using Type = Visitable<WithBaseClass<>,WithVirtualBaseClass<Bases...> , typename ToWithVisitableBaseClass<BaseTypeList>::Type, BaseTypeList>;
  };
  
  template <class T,typename ... Bases> class DerivedVisitable<T,WithVisitableBaseClass<Bases...>>:public DerivedVisitable<T,WithVirtualBaseClass<Bases...>,WithVisitableBaseClass<Bases...>>{  };
  template <typename ... Bases> class DerivedVisitable<WithVisitableBaseClass<Bases...>>:public DerivedVisitable<WithVirtualBaseClass<Bases...>,WithVisitableBaseClass<Bases...>>{  };
  
  template <class T,typename ... Bases> using DVisitable = typename DerivedVisitable<T,WithVisitableBaseClass<Bases...>>::Type;
  
  template <class T,typename ... Bases> using MVisitable = Visitable< WithBaseClass<T>, WithVirtualBaseClass<> , WithVisitableBaseClass<T,Bases...>, visitor_helper::OrderedTypeSet<visitor_helper::OrderedType<Bases, 0>...> >;

  
  
#pragma mark -
#pragma mark visitor cast
  
  template <class T> T * visitor_cast(VisitableBase *base){
    struct CastVisitor:public lars::RecursiveVisitor<T>{
      T * result = nullptr;
      bool visit(T & t) override { result = &t; return false; }
    } visitor;
    base->accept(visitor);
    return visitor.result;
  }
  
  template <class T> const T * visitor_cast(const VisitableBase *base){
    struct CastVisitor:public lars::RecursiveConstVisitor<T>{
      const T * result = nullptr;
      bool visit(const T & t) override { result = &t; return false; }
    } visitor;
    base->accept(visitor);
    return visitor.result;
  }
  
  template <class T> using is_visitable = std::is_base_of<VisitableBase, T>;
  
}
