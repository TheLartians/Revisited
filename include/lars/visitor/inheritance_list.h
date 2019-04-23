#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <lars/type_index.h>

namespace lars {
  
  namespace NEW {
  
    class SingleVisitorBase{
    public:
      virtual ~SingleVisitorBase(){}
    };

    template <class T> class SingleVisitor;
    
    class VisitorBase {
    protected:
      virtual SingleVisitorBase * getVisitorFor(const lars::TypeIndex &) = 0;
      
    public:
      
      template <class T> SingleVisitor<T> * asVisitorFor(){
        return static_cast<SingleVisitor<T>*>(getVisitorFor(get_type_index<T>()));
      }
      
      virtual ~VisitorBase(){}
    };
    
    class VisitableBase {
    public:
      virtual bool accept(VisitorBase &visitor, bool permissive = false) = 0;
      virtual bool accept(VisitorBase &visitor, bool permissive = false) const = 0;
      virtual ~VisitableBase(){}
    };
    
    template <class T> class SingleVisitor: public SingleVisitorBase {
    public:
      virtual void visit(T &v) = 0;
    };
    
    template <typename ... Args> class Visitor: public VisitorBase, public SingleVisitor<Args> ... {
      
      template <class First, typename ... Rest> SingleVisitorBase * getVisitorForWorker(const lars::TypeIndex &idx){
        if (idx == get_type_index<First>()) {
          return static_cast<SingleVisitor<First>*>(this);
        }
        if constexpr (sizeof...(Rest) > 0){
          return getVisitorForWorker<Rest...>(idx);
        }
        return nullptr;
      }
      
      SingleVisitorBase * getVisitorFor(const lars::TypeIndex &idx) override {
        return getVisitorForWorker<Args...>(idx);
      }
      
    };
    
    class IncompatibleVisitorException: public std::exception {
    private:
      mutable std::string buffer;
      
    public:
      TypeIndex typeIndex;
      IncompatibleVisitorException(TypeIndex t): typeIndex(t){}
      
      const char * what() const noexcept override {
        if (buffer.size() == 0){
          auto typeName = std::string(typeIndex.name().begin(),typeIndex.name().end());
          buffer = "IncompatibleVisitor: invalid visitor for " + typeName;
        }
        return buffer.c_str();
      }
      
    };
    
/**
 * Visitable
 */

    template <class T, class V> static bool visit(V *visitable, VisitorBase &visitor, bool permissive) {
      if (auto *v = visitor.asVisitorFor<T>()) {
        v->visit(static_cast<T&>(*visitable));
        return true;
      } else if (permissive) {
        return false;
      } else {
        throw IncompatibleVisitorException(get_type_index<V>());
      }
    }

    template <class T> class Visitable: public virtual VisitableBase {
    public:
      
      bool accept(VisitorBase &visitor, bool permissive) override {
        return visit<T>(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return visit<const T>(this, visitor, permissive);
      }
      
    };
    
/**
 * Derived Visitable
 */
    
    template <class T, class B, class V> bool visitDerived(V * v, VisitorBase &visitor, bool permissive) {
      if(visit<T>(v, visitor, true)) {
        return true;
      } else if (v->B::accept(visitor, true)) {
        return true;
      } else if (permissive) {
        return false;
      } else {
        throw IncompatibleVisitorException(get_type_index<T>());
      }
    }
    
    template <class T, class B> class DerivedVisitable: public B {
    public:
      template <typename ... Args> DerivedVisitable(Args ... args):B(args...){ }
      
      bool accept(VisitorBase &visitor, bool permissive) override {
        return visitDerived<T, B>(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return visitDerived<const T, const B>(this, visitor, permissive);
      }

    };
    
/**
 * Joint Visitable
 */
    
    template <class T, typename ... Rest, class V> bool visitJoint(V * v, VisitorBase &visitor, bool permissive) {
      if(visit<T>(v, visitor, true)) {
        return true;
      } else if constexpr (sizeof...(Rest) > 0) {
        if (visitJoint<Rest...>(v, visitor, true)) {
          return true;
        }
      }
      if (permissive) {
        return false;
      } else {
        throw IncompatibleVisitorException(get_type_index<V>());
      }
    }
    
    template <typename ... Bases> class JointVisitable: public Bases ... {
    public:
      
      bool accept(VisitorBase &visitor, bool permissive) override {
        return visitJoint<Bases...>(this, visitor, permissive);
      }
      
      bool accept(VisitorBase &visitor, bool permissive) const override {
        return visitJoint<const Bases...>(this, visitor, permissive);
      }

    };
  }
  
}
