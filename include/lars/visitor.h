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
  
  /**
   * The Visitor Prototype class. Visitors defined below are specializations of this class.
   */
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
    
  public:
    
    SingleBase * getVisitorFor(const lars::TypeIndex &idx) override {
      if constexpr (sizeof...(Args  ) > 0) {
        return getVisitorForWorker<Args...>(idx);
      } else {
        return nullptr;
      }
    }
    
  };
  
  template <class T> class SingleVisitor: public SingleVisitorBase {
  public:
    /**
     * The visit method of a regular visitor.
     * @param - The object beeing visited
     */
    virtual void visit(T) = 0;
  };
  
  using VisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleVisitor>;
  
  /**
   * A regular visitor class.
   * All types that are visitable by this class are provided in the template arguments,
   * usually as references or const references.
   * When accepted, the first matching visit method will be called.
   * If no matching visit method exists, a `InvalidVisitorException` will be thrown.
   */
  template <typename ... Args> using Visitor = VisitorPrototype<
    SingleVisitorBase,
    SingleVisitor,
    Args...
  >;
  
  template <class T> class SingleRecursiveVisitor: public SingleVisitorBase {
  public:
    /**
     * The visit method of a recursive visitor.
     * @param - The object beeing visited
     * @return - `true`, if the visitor has finished visiting and no further visit
     * methods should be called. `false`, otherwise.
     */
    virtual bool visit(T) = 0;
  };
  
  /**
   * A recursive visitor class.
   * All types that are visitable by this class are provided in the template arguments,
   * usually as references or const references.
   * When accepted, all first matching visit methods will be called until the return
   * value of a visit method is `true`.
   */
  using RecursiveVisitorBase = VisitorBasePrototype<SingleVisitorBase, SingleRecursiveVisitor>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<SingleVisitorBase, SingleRecursiveVisitor, Args...>;

  /**
   * InvalidVisitorException.
   * This exception is raised when no applicable visitor has been found.
   * It is also raisde by `visitor_cast` methods.
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
   * The Visitable base class.
   * All visitable objects are derived from this class.
   */
  class VisitableBase {
  public:
    virtual NamedTypeIndex namedTypeIndex()const = 0;
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(VisitorBase &visitor) const = 0;
    virtual bool accept(RecursiveVisitorBase &) = 0;
    virtual bool accept(RecursiveVisitorBase &) const = 0;
    virtual ~VisitableBase(){}
  };
  
  /**
   * The regular visitor algorithm.
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
  
  /**
   * The recursive visitor algorithm.
   */
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
   * An "empty" visitable object. When visited, no matching visitor methods will be found.
   */
  class EmptyVisitable: public VisitableBase {
  public:
    void accept(VisitorBase &v) override { visit(this, TypeList<>(), v); }
    void accept(VisitorBase &v) const override { visit(this, TypeList<>(), v); }
    bool accept(RecursiveVisitorBase &) override { return false; }
    bool accept(RecursiveVisitorBase &) const override { return false; }
    NamedTypeIndex namedTypeIndex() const override { return getNamedTypeIndex<EmptyVisitable>(); }
  };
  
  /**
   * A visitable object with no visitable base classes should be derived from this class
   * using itself as the template argument.
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
    
    NamedTypeIndex namedTypeIndex() const override {
      return getNamedTypeIndex<Visitable>();
    }
    
  };
  
  /**
   * A visitable object that is derived from `B` should be derived from this class
   * where `T` is the class itself. It will then be indirectly derived from `B` and
   * can receive visitors visiting `B`. Constructor arguments will be forwarded to `B`.
   * When visiting, the outermost class `T` will be visited before `B`.
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
    
    NamedTypeIndex namedTypeIndex() const override {
      return getNamedTypeIndex<DerivedVisitable>();
    }

  };
  
  /**
   * A visitable class that is derived from all classes provided by `Bases`.
   * The bases will be visited in in the order provided.
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
    
    NamedTypeIndex namedTypeIndex() const override {
      return getNamedTypeIndex<JoinVisitable>();
    }

  };

  /**
   * A visitable class that is virtually derived from all classes provided by `Bases`.
   * The bases will be visited in in the order provided, unless a base class is already
   * added further down the dependency tree.
   */
  template <typename ... Bases> class VirtualVisitable: public virtual Bases ... {
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
    
    NamedTypeIndex namedTypeIndex() const override {
      return getNamedTypeIndex<VirtualVisitable>();
    }
    
  };

  /**
   * A visitable object holding data of type `T`.
   * The template paramters `Types` and `ConstTypes` are TypeLists defining the
   * types that are visitable. `ConstTypes` is used when the accepting object is
   * const, otherwise `Types`. When accepting a visitor, `data` will be statically
   * casted to the according type.
   */
  template <class T, class Types, class ConstTypes> class DataVisitable: public virtual VisitableBase {
  public:
    T data;
    
    template <typename ... Args> DataVisitable(Args && ... args):data(std::forward<Args>(args)...){}
    
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
    
    NamedTypeIndex namedTypeIndex() const override {
      return getNamedTypeIndex<DataVisitable>();
    }

    operator T & () {
      return data;
    }
    
    operator const T & () const {
      return data;
    }
  };
  
  template <class T, typename ... Bases> using DataVisitableWithBases = DataVisitable<
    T,
    TypeList<T&, Bases &...>,
    TypeList<const T &, const Bases &..., T, Bases...>
  >;
  
  template <class T> struct PointerCastVisitor: public RecursiveVisitor<typename std::remove_pointer<T>::type &> {
    T result;
    bool visit(typename std::remove_pointer<T>::type & t) { result = &t; return true; }
  };
  
  template <class T> struct CastVisitor: public Visitor<T> {
    std::optional<T> result;
    void visit(T t) { result = t; }
  };
  
  /**
   * Casts a const reference of a visitable type to the type `T` using the visitor pattern.
   * @param - a pointer to an object derived from VisitableBase.
   * @return - if the object accepts `Visitor<T>`, then the argument of the visit method
   * is returned. Otherwise raises an `InvalidVisitorException`.
   */
  template <class T> T visitor_cast(const VisitableBase & v) {
    CastVisitor<T> visitor;
    v.accept(visitor);
    return *visitor.result;
  }

  /**
   * Casts a pointer of a visitable type to the pointer type `T` using the visitor pattern.
   * @param - a pointer to an object derived from VisitableBase.
   * @return - if the object accepts `Visitor<std::remove_pointer<T>::type &>`,
   * then a pointer to the resulting argument is called. Otherwise raises an
   * `InvalidVisitorException`
   */
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
  
  /**
   * Casts a const pointer of a visitable type to the const pointer type `T` using
   * the visitor pattern.
   * @param - a pointer to an object derived from VisitableBase.
   * @return - if the object accepts `Visitor<std::remove_pointer<T>::type &>`,
   * then a pointer to the resulting argument is called. Otherwise raises an
   * `InvalidVisitorException`
   */
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

  /**
   * Casts a reference of a visitable type to the type `T` using the visitor pattern.
   * @param - a pointer to an object derived from VisitableBase.
   * @return - if the object accepts `Visitor<T>`, then the argument of the visit method
   * is returned. Otherwise raises an `InvalidVisitorException`.
   */
  template <class T> typename std::enable_if<
    std::is_reference<T>::value,
    T
  >::type visitor_cast(VisitableBase & v) {
    if (auto res = visitor_cast<typename std::remove_reference<T>::type *>(&v)) {
      return *res;
    } else {
      throw InvalidVisitorException(v.namedTypeIndex());
    }
  }
  
  
}
