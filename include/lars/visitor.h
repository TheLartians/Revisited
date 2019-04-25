#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <optional>
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
    virtual void visit(T v) = 0;
  };
  
  using VisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleVisitor>;
  template <typename ... Args> using Visitor = VisitorPrototype<SingleVisitorBase, SingleVisitor, Args...>;
  
  /**
   * Recursive Visitor
   */
  
  template <class T> class SingleRecursiveVisitor: public SingleVisitorBase {
  public:
    virtual bool visit(T v) = 0;
  };
  
  using RecursiveVisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleRecursiveVisitor>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<SingleVisitorBase, SingleRecursiveVisitor, Args...>;

  /**
   * Errors
   */
  
  class InvalidVisitorException: public std::exception {
  private:
    mutable std::string buffer;
    
  public:
    NamedTypeIndex typeIndex;
    InvalidVisitorException(NamedTypeIndex t): typeIndex(t){}
    
    const char * what() const noexcept override {
      if (buffer.size() == 0){
        buffer = "invalid visitor for " + typeIndex.name();
      }
      return buffer.c_str();
    }
  };
  
  /**
   * VisitableBase
   */
  
  class VisitableBase {
  public:
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(VisitorBase &visitor) const = 0;
    virtual bool accept(RecursiveVisitorBase &) = 0;
    virtual bool accept(RecursiveVisitorBase &) const = 0;
    virtual ~VisitableBase(){}
  };
  
  /**
   * Visit algorithms
   */

  template <class V, class T, typename ... Rest> static void visit(
    V * visitable,
    TypeList<T, Rest...>,
    VisitorBase &visitor
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      v->visit(static_cast<T>(*visitable));
    } else if constexpr (sizeof...(Rest) > 0) {
      visit(visitable, TypeList<Rest...>(), visitor);
    } else {
      throw InvalidVisitorException(getNamedTypeIndex<V>());
    }
  }
  
  template <class V> static bool visit(V *, TypeList<>, VisitorBase &) {
    throw InvalidVisitorException(getNamedTypeIndex<V>());
  }
  
  template <class V, class T, typename ... Rest> static bool visit(
    V * visitable,
    TypeList<T, Rest...>,
    RecursiveVisitorBase &visitor
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      if (v->visit(static_cast<T>(*visitable))) {
        return true;
      }
    }
    if constexpr (sizeof...(Rest) > 0) {
      return visit(visitable, TypeList<Rest...>(), visitor);
    } else {
      return false;
    }
  }
  
  template <class V> static bool visit(V *, TypeList<>, RecursiveVisitorBase &) {
    return false;
  }
  
  /**
   * NonVisitable
   */
  
  class EmptyVisitable: public VisitableBase {
  public:
    void accept(VisitorBase &v) override { visit(this, TypeList<>(), v); }
    void accept(VisitorBase &v) const override { visit(this, TypeList<>(), v); }
    bool accept(RecursiveVisitorBase &) override { return false; }
    bool accept(RecursiveVisitorBase &) const override { return false; }
  };
  
  /**
   * Visitable
   */
  
  template <class T> class Visitable: public virtual VisitableBase {
  public:
    
    using InheritanceList = lars::InheritanceList<OrderedType<T, 0>>;

    void accept(VisitorBase &visitor) override {
      visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    void accept(VisitorBase &visitor) const override {
      visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
  };
  
  /**
   * Derived Visitable
   */
  
  template <class T, class B> class DerivedVisitable: public B {
  public:
    
    using InheritanceList = typename B::InheritanceList::template Push<T>;

    void accept(VisitorBase &visitor) override {
      visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    void accept(VisitorBase &visitor) const override {
      visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }

  };
  
  /**
   * Join Visitable
   */
  
  template <typename ... Bases> class JoinVisitable: public Bases ... {
  public:
    
    using InheritanceList = lars::InheritanceList<>::Merge<typename Bases::InheritanceList ...>;

    void accept(VisitorBase &visitor) override {
      visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    void accept(VisitorBase &visitor) const override {
      visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }

  };

  /**
   * Virtual Join Visitable
   */
  
  template <typename ... Bases> class VirtualJoinVisitable: public virtual Bases ... {
  public:
    
    using InheritanceList = lars::InheritanceList<>::Merge<typename Bases::InheritanceList ...>;
    
    void accept(VisitorBase &visitor) override {
      visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    void accept(VisitorBase &visitor) const override {
      visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, typename InheritanceList::ReferenceTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, typename InheritanceList::ConstReferenceTypes(), visitor);
    }
    
  };

  /**
   * Data Visitable
   */
  
  template <class T, class Types, class ConstTypes> class DataVisitable: public virtual VisitableBase {
  public:
    T data;
    
    template <typename ... Args> DataVisitable(Args && ... args):data(args...){}
    
    void accept(VisitorBase &visitor) override {
      visit(this, Types(), visitor);
    }
    
    void accept(VisitorBase &visitor) const override {
      visit(this, ConstTypes(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) override {
      return visit(this, Types(), visitor);
    }
    
    bool accept(RecursiveVisitorBase &visitor) const override {
      return visit(this, ConstTypes(), visitor);
    }
    
    template <
      class O,
    typename = typename std::enable_if<std::is_same<T,O>::value || std::is_base_of<O, T>::value>::type
    > operator O & () {
      return data;
    }
    
    template <
      class O,
      typename = typename std::enable_if<std::is_same<T,O>::value || std::is_base_of<O, T>::value>::type
    > operator const O & () const {
      return data;
    }
    
    template <
      class O,
      typename = typename std::enable_if<!(std::is_same<T,O>::value || std::is_base_of<O, T>::value)>::type
    > operator O () const {
      return data;
    }

  };
  
  /**
   * Visitor cast
   */

  template <class T> struct PointerCastVisitor: public RecursiveVisitor<typename std::remove_pointer<T>::type &> {
    T result;
    bool visit(typename std::remove_pointer<T>::type & t) { result = &t; return true; }
  };
  
  template <class T> typename std::enable_if<
    std::is_pointer<T>::value
    && !std::is_const<typename std::remove_pointer<T>::type>::value,
    T
  >::type visitor_cast(VisitableBase * v) {
    PointerCastVisitor<T> visitor;
    if (v->accept(visitor)) {
      return visitor.result;
    } else {
      return nullptr;
    }
  }
  
  template <class T> typename std::enable_if<
    std::is_pointer<T>::value
    && std::is_const<typename std::remove_pointer<T>::type>::value,
    T
  >::type visitor_cast(const VisitableBase * v) {
    PointerCastVisitor<T> visitor;
    if (v->accept(visitor)) {
      return visitor.result;
    } else {
      return nullptr;
    }
  }

  template <class T, class V> typename std::enable_if<
    std::is_reference<T>::value,
    T
  >::type visitor_cast(V & v) {
    if (auto res = visitor_cast<typename std::remove_reference<T>::type *>(&v)) {
      return *res;
    } else {
      throw InvalidVisitorException(getNamedTypeIndex<T>());
    }
  }
  
  template <class T> struct CastVisitor: public Visitor<T> {
    std::optional<T> result;
    void visit(T t) { result = t; }
  };
  
  template <class T, class V> typename std::enable_if<
    !std::is_reference<T>::value,
    T
  >::type visitor_cast(V & v) {
    CastVisitor<T> visitor;
    v.accept(visitor);
    return *visitor.result;
  }
  
}
