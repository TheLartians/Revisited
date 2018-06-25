#pragma once
#include <lars/visitor.h>

namespace lars{
  
  template <class T> std::shared_ptr<T> shared_visitor_cast(const std::shared_ptr<VisitableBase> &base){
    struct CastVisitor:public lars::RecursiveVisitor<T>{
      T * result = nullptr;
      bool visit(T & t) override { result = &t; return false; }
    } visitor;
    base->accept(visitor);
    return std::shared_ptr<T>(base,visitor.result);
  }
  
  template <class T> std::shared_ptr<const T> const_shared_visitor_cast(const std::shared_ptr<const VisitableBase> &base){
    struct CastVisitor:public lars::RecursiveConstVisitor<T>{
      const T * result = nullptr;
      bool visit(const T & t) override { result = &t; return false; }
    } visitor;
    base->accept(visitor);
    return std::shared_ptr<const T>(base,visitor.result);
  }
  
}
