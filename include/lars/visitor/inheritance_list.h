#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <lars/type_index.h>

// #define LARS_VISITOR_DEBUG

#ifdef LARS_VISITOR_DEBUG
#include <lars/log.h>
std::string LARS_VISITOR_LOG_INDENT;
#define LARS_VISITOR_LOG(X) LARS_LOG(LARS_VISITOR_LOG_INDENT << X)
#define LARS_VISITOR_INCREASE_INDENT LARS_VISITOR_LOG_INDENT += "  "
#define LARS_VISITOR_DECREASE_INDENT LARS_VISITOR_LOG_INDENT = LARS_VISITOR_LOG_INDENT.substr(2)
struct LARS_VISITOR_WITH_INDENT_KEEPER {
  LARS_VISITOR_WITH_INDENT_KEEPER(){ LARS_VISITOR_INCREASE_INDENT; }
  ~LARS_VISITOR_WITH_INDENT_KEEPER(){ LARS_VISITOR_DECREASE_INDENT; }
};
#define LARS_VISITOR_WITH_INDENT() LARS_VISITOR_LOG(get_type_name<decltype(*v)>() << " accepting visitor"); LARS_VISITOR_WITH_INDENT_KEEPER __LARS_VISITOR_INDENT_KEEPER
#else
#define LARS_VISITOR_LOG(X)
#define LARS_VISITOR_WITH_INDENT()
#endif

namespace lars {
  
  namespace NEW {

    /**
     * Visitor Prototype
     */
    
    template <class T> class SingleVisitor;
    
    template <class SingleBase, template <class T> class Single> class VisitorBasePrototype {
    protected:
      virtual SingleBase * getVisitorFor(const lars::TypeIndex &) = 0;
      
    public:
      
      template <class T> Single<T> * asVisitorFor(){
        return static_cast<Single<T>*>(getVisitorFor(get_type_index<T>()));
      }
      
      virtual ~VisitorBasePrototype(){}
    };
    
    class SingleVisitorBase{
    public:
      virtual ~SingleVisitorBase(){}
    };

    template <class SingleBase, template <class T> class Single,typename ... Args> class VisitorPrototype: public virtual VisitorBasePrototype<SingleBase, Single>, public Single<Args> ... {
      
      template <class First, typename ... Rest> inline SingleBase * getVisitorForWorker(const lars::TypeIndex &idx){
        if (idx == get_type_index<First>()) {
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
     * Errors
     */
    
    class IncompatibleVisitorException: public std::exception {
    private:
      mutable std::string buffer;
      
    public:
      TypeIndex typeIndex;
      IncompatibleVisitorException(TypeIndex t): typeIndex(t){}
      
      const char * what() const noexcept override {
        if (buffer.size() == 0){
          auto typeName = std::string(typeIndex.name().begin(),typeIndex.name().end());
          buffer = "IncompatibleVisitor: incompatible visitor for " + typeName;
        }
        return buffer.c_str();
      }
    };

    /**
     * Visitable
     */

    class VisitableBase {
    public:
      virtual bool accept(VisitorBase &visitor, bool permissive = false) = 0;
      virtual bool accept(VisitorBase &visitor, bool permissive = false) const = 0;
      virtual bool accept(RecursiveVisitorBase &) = 0;
      virtual bool accept(RecursiveVisitorBase &) const = 0;
      virtual ~VisitableBase(){}
    };

    template <class T, class V> static bool visit(V *visitable, VisitorBase &visitor, bool permissive) {
      LARS_VISITOR_LOG("visit single " << lars::get_type_name<T>());
      if (auto *v = visitor.asVisitorFor<T>()) {
        v->visit(static_cast<T&>(*visitable));
        return true;
      }
      if (permissive) {
        return false;
      }
      throw IncompatibleVisitorException(get_type_index<V>());
    }

    template <class T, class V> static bool visit(V *visitable, RecursiveVisitorBase &visitor) {
      LARS_VISITOR_LOG("recursive visit single " << lars::get_type_name<T>());
      if (auto *v = visitor.asVisitorFor<T>()) {
        return v->visit(static_cast<T&>(*visitable));
      }
      return true;
    }

    template <class T> class Visitable: public virtual VisitableBase {
    public:
      
      template <typename ... Args> static bool staticAccept(Visitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visit<T>(v, args...);
      }
      
      template <typename ... Args> static bool staticAccept(const Visitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visit<const T>(v, args...);
      }

      bool accept(VisitorBase &visitor, bool permissive) override {
        return staticAccept(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return staticAccept(this, visitor, permissive);
      }
      
      bool accept(RecursiveVisitorBase &visitor) override {
        return staticAccept(this, visitor);
      }
      
      bool accept(RecursiveVisitorBase &visitor) const override {
        return staticAccept(this, visitor);
      }
      
    };
    
    /**
     * Derived Visitable
     */
    
    template <class T, class B, class V> bool visitDerived(V * v, VisitorBase &visitor, bool permissive) {
      LARS_VISITOR_LOG("visit derived " << lars::get_type_name<T>() << " with base " << lars::get_type_name<B>());
      if(visit<T>(v, visitor, true)) {
        return true;
      }
      if (B::staticAccept(v,visitor, true)) {
        return true;
      }
      if (permissive) {
        return false;
      }
      throw IncompatibleVisitorException(get_type_index<T>());
    }
    
    template <class T, class B, class V> bool visitDerived(V * v, RecursiveVisitorBase &visitor) {
      LARS_VISITOR_LOG("recursive visit derived type " << lars::get_type_name<T>());
      if(!visit<T>(v, visitor)) {
        return false;
      }
      LARS_VISITOR_LOG("recursive visit derived base " << lars::get_type_name<B>());
      if (!B::staticAccept(v, visitor)) {
        return false;
      }
      return true;
    }

    template <class T, class B> class DerivedVisitable: public B {
    public:
      template <typename ... Args> DerivedVisitable(Args ... args):B(args...){ }
      
      template <typename ... Args> static bool staticAccept(DerivedVisitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visitDerived<T,B>(v, args...);
      }
      
      template <typename ... Args> static bool staticAccept(const DerivedVisitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visitDerived<const T,const B>(v, args...);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) override {
        return staticAccept(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return staticAccept(this, visitor, permissive);
      }

      bool accept(RecursiveVisitorBase &visitor) override {
        return staticAccept(this, visitor);
      }
      
      bool accept(RecursiveVisitorBase &visitor) const override {
        return staticAccept(this, visitor);
      }

    };
    
    /**
     * Joint Visitable
     */
    
    template <class T, typename ... Rest, class V> bool visitJoint(V * v, VisitorBase &visitor, bool permissive) {
      LARS_VISITOR_LOG("visit join " << lars::get_type_name<V>());
      if(T::staticAccept(v, visitor, true)) {
        return true;
      }
      if constexpr (sizeof...(Rest) > 0) {
        if (visitJoint<Rest...>(v, visitor, true)) {
          return true;
        }
      }
      if (permissive) {
        return false;
      }
      throw IncompatibleVisitorException(get_type_index<V>());
    }
    
    template <class T, typename ... Rest, class V> bool visitJoint(V * v, RecursiveVisitorBase &visitor) {
      LARS_VISITOR_LOG("recursive visit join " << lars::get_type_name<V>() << ": " << lars::get_type_name<T>());
      if(!T::staticAccept(v, visitor)) {
        return false;
      }
      if constexpr (sizeof...(Rest) > 0) {
        if (!visitJoint<Rest...>(v, visitor)) {
          return false;
        }
      }
      return true;
    }
    
    template <typename ... Bases> class JointVisitable: public Bases ... {
    public:
      
      template <typename ... Args> static bool staticAccept(JointVisitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visitJoint<Bases...>(v, args...);
      }
      
      template <typename ... Args> static bool staticAccept(const JointVisitable *v,Args && ... args){
        LARS_VISITOR_WITH_INDENT();
        return visitJoint<const Bases...>(v, args...);
      }

      bool accept(VisitorBase &visitor, bool permissive) override {
        return staticAccept(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return staticAccept(this, visitor, permissive);
      }
      
      bool accept(RecursiveVisitorBase &visitor) override {
        return staticAccept(this, visitor);
      }
      
      bool accept(RecursiveVisitorBase &visitor) const override {
        return staticAccept(this, visitor);
      }

    };
  }
  
}
