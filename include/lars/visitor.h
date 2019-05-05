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
  public:
    virtual SingleBase * getVisitorFor(const lars::StaticTypeIndex &) = 0;
    
    template <class T> Single<T> * asVisitorFor(){
      return static_cast<Single<T>*>(getVisitorFor(getStaticTypeIndex<T>()));
    }
    
    virtual ~VisitorBasePrototype(){}
  };
    
  /**
   * The Visitor Prototype class. Visitors defined below are specializations of this class.
   */
  template <class SingleBase, template <class T> class Single,typename ... Args> class VisitorPrototype: public virtual VisitorBasePrototype<SingleBase, Single>, public Single<Args> ... {
  private:
    template <class First, typename ... Rest> inline SingleBase * getVisitorForWorker(const lars::StaticTypeIndex &idx){
      if (idx == getStaticTypeIndex<First>()) {
        return static_cast<Single<First>*>(this);
      }
      if constexpr (sizeof...(Rest) > 0){
        return getVisitorForWorker<Rest...>(idx);
      }
      return nullptr;
    }
  
  public:

    SingleBase * getVisitorFor(const lars::StaticTypeIndex &idx) override {
      if constexpr (sizeof...(Args  ) > 0) {
        return getVisitorForWorker<Args...>(idx);
      } else {
        return nullptr;
      }
    }
  
  };

  class SingleVisitorBase{
  public:
    virtual ~SingleVisitorBase(){}
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
  
  class SingleRecursiveVisitorBase{
  public:
    virtual ~SingleRecursiveVisitorBase(){}
  };
  
  template <class T> class SingleRecursiveVisitor: public SingleRecursiveVisitorBase {
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
  using RecursiveVisitorBase = VisitorBasePrototype<SingleRecursiveVisitorBase, SingleRecursiveVisitor>;
  template <typename ... Args> using RecursiveVisitor = VisitorPrototype<SingleRecursiveVisitorBase, SingleRecursiveVisitor, Args...>;

  /**
   * InvalidVisitorException.
   * This exception is raised when no applicable visitor has been found.
   * It is also raisde by `visitor_cast` methods.
   */
  class InvalidVisitorException: public std::exception {
  private:
    mutable std::string buffer;
    
  public:
    TypeIndex StaticTypeIndex;
    InvalidVisitorException(TypeIndex t): StaticTypeIndex(t){}
    
    const char * what() const noexcept override {
      if (buffer.size() == 0){
        buffer = "invalid visitor for " + StaticTypeIndex.name();
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
    virtual TypeIndex StaticTypeIndex()const = 0;
    virtual void accept(VisitorBase &visitor) = 0;
    virtual void accept(VisitorBase &visitor) const = 0;
    virtual bool accept(RecursiveVisitorBase &) = 0;
    virtual bool accept(RecursiveVisitorBase &) const = 0;
    virtual ~VisitableBase(){}
  };
  
  /**
   * Base class for indirect visitable objects.
   * Must implement a `cast<T>()` method.
   */
  struct IndirectVisitableBase { };

  /**
   * The regular visitor algorithm.
   */
  template <class V, class T, typename ... Rest> static void visit(
    V * visitable,
    TypeList<T, Rest...>,
    VisitorBase &visitor
  ) {
    if (auto *v = visitor.asVisitorFor<T>()) {
      if constexpr (std::is_base_of<IndirectVisitableBase, typename std::decay<V>::type>::value){
        v->visit(visitable->template cast<T>());
      } else {
        v->visit(static_cast<T>(*visitable));
      }
    } else if constexpr (sizeof...(Rest) > 0) {
      visit(visitable, TypeList<Rest...>(), visitor);
    } else {
      throw InvalidVisitorException(getTypeIndex<V>());
    }
  }
  
  template <class V> static bool visit(V *, TypeList<>, VisitorBase &) {
    throw InvalidVisitorException(getTypeIndex<V>());
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
      if constexpr (std::is_base_of<IndirectVisitableBase, typename std::decay<V>::type>::value){
        if (v->visit(visitable->template cast<T>())) {
          return true;
        }
      } else {
        if (v->visit(static_cast<T>(*visitable))) {
          return true;
        }
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
    using Type = EmptyVisitable;
    using Types = TypeList<>;
    using ConstTypes = TypeList<>;

    void accept(VisitorBase &v) override { visit(this, Types(), v); }
    void accept(VisitorBase &v) const override { visit(this, ConstTypes(), v); }
    bool accept(RecursiveVisitorBase &) override { return false; }
    bool accept(RecursiveVisitorBase &) const override { return false; }
    TypeIndex StaticTypeIndex() const override { return getTypeIndex<void>(); }
  };
  
  /**
   * A visitable object with no visitable base classes should be derived from this class
   * using itself as the template argument.
   */
  template <class T> class Visitable: public virtual VisitableBase {
  public:
    using InheritanceList = lars::InheritanceList<OrderedType<T, 0>>;
    using Type = T;
    using Types = typename InheritanceList::ConvertibleTypes;
    using ConstTypes = typename InheritanceList::ConstConvertibleTypes;

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
    
    TypeIndex StaticTypeIndex() const override {
      return getTypeIndex<T>();
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
    using Type = T;
    using Types = typename InheritanceList::ConvertibleTypes;
    using ConstTypes = typename InheritanceList::ConstConvertibleTypes;

    template <typename ... Args> DerivedVisitable(Args && ... args):B(std::forward<Args>(args)...) {}
    using B::accept;

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
    
    TypeIndex StaticTypeIndex() const override {
      return getTypeIndex<T>();
    }

  };
  
  /**
   * A visitable class that is derived from all classes provided by `Bases`.
   * The bases will be visited in in the order provided.
   */
  template <typename ... Bases> class JoinVisitable: public Bases ... {
  public:
    using InheritanceList = lars::InheritanceList<>::Merge<typename Bases::InheritanceList ...>;
    using Type = JoinVisitable;
    using Types = typename InheritanceList::ConvertibleTypes;
    using ConstTypes = typename InheritanceList::ConstConvertibleTypes;

    using Bases::accept ...;
    
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
    
    TypeIndex StaticTypeIndex() const override {
      return getTypeIndex<JoinVisitable>();
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
    using Type = VirtualVisitable;
    using Types = typename InheritanceList::ConvertibleTypes;
    using ConstTypes = typename InheritanceList::ConstConvertibleTypes;

    using Bases::accept ...;
    
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
    
    TypeIndex StaticTypeIndex() const override {
      return getTypeIndex<VirtualVisitable>();
    }
    
  };

  /**
   * Prototype for a visitable object holding data of type `T`.
   * The template paramters `Types` and `ConstTypes` are TypeLists defining the
   * types that are visitable. `ConstTypes` is used when the accepting object is
   * const, otherwise `Types`. When accepting a visitor, `data` will be statically
   * casted to the according type.
   */
  template <
    class T,
    class _Types,
    class _ConstTypes,
    class BaseCast = T
  > class DataVisitablePrototype: public virtual VisitableBase, public IndirectVisitableBase {
  public:
    using Type = BaseCast;
    using Types = _Types;
    using ConstTypes = _ConstTypes;
    
    T data;

    template <typename ... Args> DataVisitablePrototype(Args && ... args):data(std::forward<Args>(args)...){}
    
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
    
    TypeIndex StaticTypeIndex() const override {
      return getTypeIndex<T>();
    }
    
    template <typename O> O cast(){
      return static_cast<O>(data);
    }
    
    template <typename O> O cast() const {
      return static_cast<O>(data);
    }
    
    operator BaseCast &() {
      return data;
    }

    operator const BaseCast &() const {
      return data;
    }

  };
  
  template <class T, class B, class C, class CastType> class DataVisitableWithBasesAndConversionsDefinition;

  template <class T, typename ... Bases, typename ... Conversions, class CastType> class DataVisitableWithBasesAndConversionsDefinition<
    T, 
    TypeList<Bases...>, 
    TypeList<Conversions...>, 
    CastType
  >{
  private:
    using ConstTypes = typename TypeList<const T &, const Bases &...,Conversions...>::template Merge<
      typename TypeList<T,Bases...>::template Filter<std::is_copy_constructible>
    >;

  public:
    using type = DataVisitablePrototype<
      T,
      typename TypeList<T&, Bases &...>::template Merge<ConstTypes>,
      ConstTypes,
      CastType
    >;
  };

  /**
   * A visitable object holding data of type `T` with bases defined by the `Bases` typelist that can also be converted to `Conversions`.
   * By default it is castable to all reference and const reference types as well as value types that are copy-constructable.
   */
  template <class T, class Bases, class Conversions, class CastType = T> using DataVisitableWithBasesAndConversions = typename DataVisitableWithBasesAndConversionsDefinition<
    T, 
    Bases, 
    Conversions,
    CastType
  >::type;

  /**
   * Same as DataVisitableWithBasesAndConversions, without conversions.
   */
  template <class T, typename ... Bases> using DataVisitableWithBases = DataVisitableWithBasesAndConversions<
    T, 
    TypeList<Bases...>, 
    TypeList<>
  >;
  
  /**
   * Same as DataVisitableWithBases, without base types.
   */
  template <class T> using DataVisitable = DataVisitableWithBases<T>;

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
    if (v && v->accept(visitor)) {
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
    if (v && v->accept(visitor)) {
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
      throw InvalidVisitorException(v.StaticTypeIndex());
    }
  }
  
}

/**
 * Macro for removing visitable methods for a class inheriting from multiple visitable classes.
 */
#define LARS_VISITOR_REMOVE_VISITABLE_METHODS \
using InheritanceList = ::lars::InheritanceList<>;\
using Type = ::lars::EmptyVisitable;\
using Types = ::lars::TypeList<>;\
using ConstTypes = ::lars::TypeList<>;\
void accept(::lars::VisitorBase &visitor)override{ throw ::lars::InvalidVisitorException(StaticTypeIndex()); }\
void accept(::lars::VisitorBase &visitor) const override { throw ::lars::InvalidVisitorException(StaticTypeIndex()); }\
bool accept(::lars::RecursiveVisitorBase &visitor) override { return false; }\
bool accept(::lars::RecursiveVisitorBase &visitor) const override { return false; }\
::lars::TypeIndex StaticTypeIndex() const override { return ::lars::getTypeIndex<::lars::EmptyVisitable>(); }

