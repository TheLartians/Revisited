#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <lars/inheritance_list.h>
#include <lars/type_index.h>

namespace lars {

  /**
   * Visitor Prototype
   */
  
  template <class T> class SingleVisitor;
  
  template <class SingleBase, template <class T> class Single> class VisitorBasePrototype {
  protected:
    virtual SingleBase * getVisitorFor(const lars::TypeIndex &) = 0;
    
  public:
    
    template <class T> Single<T> * asVisitorFor(){
      return static_cast<Single<T>*>(getVisitorFor(getTypeIndex<T>()));
    }
    
    virtual ~VisitorBasePrototype(){}
  };
  
  class SingleVisitorBase{
  public:
    virtual ~SingleVisitorBase(){}
  };
  
  template <class SingleBase, template <class T> class Single,typename ... Args> class VisitorPrototype: public virtual VisitorBasePrototype<SingleBase, Single>, public Single<Args> ... {
    
    template <class First, typename ... Rest> inline SingleBase * getVisitorForWorker(const lars::TypeIndex &idx){
      if (idx == getTypeIndex<First>()) {
        return static_cast<Single<First>*>(this);
      }
      if constexpr (sizeof...(Rest) > 0){
        return getVisitorForWorker<Rest...>(idx);
      }
      return nullptr;
    }
    
    SingleBase * getVisitorFor(const lars::TypeIndex &idx) override {
      return getVisitorForWorker<Args...>(idx);
    }
    
  };
  
  /**
   * Visitor
   */
  
  template <class T> class SingleVisitor: public SingleVisitorBase {
  public:
    virtual void visit(T &v) = 0;
  };
  
  using VisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleVisitor>;
  template <typename ... Args> using Visitor = VisitorPrototype<SingleVisitorBase, SingleVisitor, Args...>;
  
  /**
   * Recursive Visitor
   */
  
  template <class T> class SingleRecursiveVisitor: public SingleVisitorBase {
  public:
    virtual bool visit(T &v) = 0;
  };
  
  using RecursiveVisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleRecursiveVisitor>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<SingleVisitorBase, SingleRecursiveVisitor, Args...>;

  /**
   * Value Visitor
   */
  
  template <class T> class SingleValueVisitor: public SingleVisitorBase {
  public:
    virtual bool visit(T v) = 0;
  };
  
  using ValueVisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleValueVisitor>;
  template <typename ... Args> using ValueVisitor = VisitorPrototype<SingleVisitorBase, SingleValueVisitor, Args...>;

  /**
   * Errors
   */
  
  class IncompatibleVisitorException: public std::exception {
  private:
    mutable std::string buffer;
    
  public:
    NamedTypeIndex typeIndex;
    IncompatibleVisitorException(NamedTypeIndex t): typeIndex(t){}
    
    const char * what() const noexcept override {
      if (buffer.size() == 0){
        buffer = "IncompatibleVisitor: incompatible visitor for " + typeIndex.name();
      }
      return buffer.c_str();
    }
  };
  
  /**
   * VisitableBase
   */
  
  class VisitableBase {
  public:
    virtual bool accept(VisitorBase &visitor, bool permissive = false) = 0;
    virtual bool accept(VisitorBase &visitor, bool permissive = false) const = 0;
    virtual bool accept(RecursiveVisitorBase &) = 0;
    virtual bool accept(RecursiveVisitorBase &) const = 0;
    virtual bool accept(ValueVisitorBase &) const = 0;
    virtual ~VisitableBase(){}
  };
  
  template <class V> static bool visit(
    V *,
    TypeList<>,
    VisitorBase &,
    bool permissive
  ) {
    if (permissive) {
      return false;
    } else {
      throw IncompatibleVisitorException(getNamedTypeIndex<V>());
    }
  }

  template <class V, class T, typename ... Rest> static bool visit(
    V * visitable,
    TypeList<T, Rest...>,
    VisitorBase &visitor,
    bool permissive
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      v->visit(static_cast<T&>(*visitable));
      return true;
    } else {
      return visit(visitable, TypeList<Rest...>(), visitor, permissive);
    }
  }
  
  template <class V, class T, typename ... Rest> static bool visit(
    V * visitable,
    TypeList<T, Rest...>,
    RecursiveVisitorBase &visitor
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      if (!v->visit(static_cast<T&>(*visitable))) {
        return false;
      }
    }
    if constexpr (sizeof...(Rest) > 0) {
      return visit(visitable, TypeList<Rest...>(), visitor);
    } else {
      return true;
    }
  }
  
  template <class V, class T, typename ... Rest> static bool visit(
    V * visitable,
    TypeList<T, Rest...>,
    ValueVisitorBase &visitor,
    bool permissive
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      v->visit(static_cast<T>(*visitable));
      return true;
    }
    if constexpr (sizeof...(Rest) > 0) {
      return visit(visitable, TypeList<Rest...>(), visitor);
    } else if (permissive) {
      return false;
    } else {
      throw IncompatibleVisitorException(getNamedTypeIndex<V>());
    }
  }
  
  /**
   * NonVisitable
   */
  
  class NonVisitable: public VisitableBase {
  public:
    bool accept(VisitorBase &, bool) override { return false; }
    bool accept(VisitorBase &, bool) const override { return false; }
    bool accept(RecursiveVisitorBase &) override { return false; }
    bool accept(RecursiveVisitorBase &) const override { return false; }
    bool accept(ValueVisitorBase &) const override { return false; }
  };
  
  /**
   * Visitable
   */
  
  template <class T> class Visitable: public virtual VisitableBase {
  public:
    
    using InheritanceList = lars::InheritanceList<OrderedType<T, 0>>;

    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }
    
    bool accept(ValueVisitorBase &) const override { return false; }

  };
  
  /**
   * Derived Visitable
   */
  
  template <class T, class B> class DerivedVisitable: public B {
  public:
    
    using InheritanceList = typename B::InheritanceList::template Push<T>;

    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }

    bool accept(ValueVisitorBase &) const override { return false; }

  };
  
  /**
   * Join Visitable
   */
  
  template <typename ... Bases> class JoinVisitable: public Bases ... {
  public:
    
    using InheritanceList = lars::InheritanceList<>::Merge<typename Bases::InheritanceList ...>;

    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }

    bool accept(ValueVisitorBase &) const override { return false; }

  };

  /**
   * Virtual Join Visitable
   */
  
  template <typename ... Bases> class VirtualJoinVisitable: public virtual Bases ... {
  public:
    
    using InheritanceList = lars::InheritanceList<>::Merge<typename Bases::InheritanceList ...>;
    
    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }
    
    bool accept(ValueVisitorBase &) const override { return false; }
    
  };

  /**
   * Proxy Visitable
   */
  
  template <class T, class Inheritance> class ProxyVisitable: public virtual VisitableBase {
  public:
    T * data;
    
    ProxyVisitable(T * d):data(d){}
    
    using InheritanceList = Inheritance;
    
    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }
    
    template <class O> operator O & (){
      return *data;
    }
    
    template <class O> operator const O & ()const{
      return *data;
    }
    
    bool accept(ValueVisitorBase &) const override { return false; }
    
  };

  /**
   * Data Visitable
   */
  
  template <class T, class Inheritance> class DataVisitable: public virtual VisitableBase {
  public:
    T data;
    
    template <typename ... Args> DataVisitable(Args && ... args):data(args...){}
    
    using InheritanceList = Inheritance;
    
    bool accept(VisitorBase &visitor, bool permissive) override {
      return visit(this, typename InheritanceList::Types(), visitor, permissive);
    }
    
    bool accept(VisitorBase &visitor, bool permissive) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor, permissive);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstTypes(), visitor);
    }
    
    template <class O> operator O & (){
      return data;
    }
    
    template <class O> operator const O & ()const{
      return data;
    }
    
  };
  
  /**
   * Visitor cast
   */

  template <class T> struct CastVisitor: public RecursiveVisitor<T> {
    T * result = nullptr;
    bool visit(T & t) { result = &t; return false; }
  };
  
  template <class T> typename std::enable_if<!std::is_const<T>::value, T>::type * visitor_cast(VisitableBase * v) {
    CastVisitor<T> visitor;
    v->accept(visitor);
    return visitor.result;
  }
  
  template <class T> typename std::enable_if<std::is_const<T>::value, T>::type * visitor_cast(const VisitableBase * v) {
    CastVisitor<T> visitor;
    v->accept(visitor);
    return visitor.result;
  }

  template <class T, class V> T & visitor_cast(V & v) {
    if (auto res = visitor_cast<T>(&v)) {
      return *res;
    } else {
      throw IncompatibleVisitorException(getNamedTypeIndex<T>());
    }
  }
}
