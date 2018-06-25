#pragma once

#include <lars/visitor.h>
#include <lars/dummy.h>
#include <lars/iterators.h>

#include <unordered_map>
#include <vector>

namespace lars {
  
  template <class VisitorBase,template<typename ...> class Visitor> class ComponentVisitorPrototype:public VisitorBase{
  protected:
    std::unordered_map<TypeIndex,std::pair<std::shared_ptr<VisitorBase>,void*>> visitors;
  public:

    void * as_visitor_for(TypeIndex t)override{
      return visitors[t].second;
    }
    
    void add_visitor_for_type(TypeIndex t,const std::shared_ptr<VisitorBase> v){
      visitors[t] = std::make_pair(v, v->as_visitor_for(t));
    }
    
    template <class T,class F> void add_visitor(F f){
      struct SpecialVisitor:public Visitor<T>{
        F f;
        SpecialVisitor(const F & f):f(f){}
        void visit(T &t){ f(t); }
      };
      add_visitor_for_type(get_type_index<T>(), std::make_shared<SpecialVisitor>(f));
    }
    
    void remove_visitor_for_type(TypeIndex t){
      visitors.erase(visitors.find(t));
    }

    template <class T> void remove_visitor(){ remove_visitor_for_type(get_type_index<T>()); }
  };
  
  using ComponentVisitor = ComponentVisitorPrototype<VisitorBase,Visitor>;
  using ConstComponentVisitor = ComponentVisitorPrototype<ConstVisitorBase,ConstVisitor>;

  class ComponentVisitable:public VisitableBase{
  private:
    std::vector<std::pair<TypeIndex,std::shared_ptr<VisitableBase>>> components;
    
    template <bool recursive,typename Visitor,class Self> static void accept(Self &self,Visitor &visitor){
      bool accepted = false;
      for(auto && c:reversed(self.components)){
        if(auto v = visitor.as_visitor_for(c.first)){
          accepted = true;
          c.second->accept(visitor);
          if(!recursive) break;
        }
      }
      if(!recursive && !accepted) throw IncompatibleVisitorException();
    }
    
  public:
    
    void accept(VisitorBase &visitor)override{ accept<false>(*this,visitor); }
    void accept(ConstVisitorBase &visitor)const override{ accept<false>(*this,visitor); }
    void accept(RecursiveVisitorBase &visitor)override{ accept<true>(*this,visitor); }
    void accept(RecursiveConstVisitorBase &visitor)const override{ accept<true>(*this,visitor); }
    
    void add_component_for_type(TypeIndex t,const std::shared_ptr<VisitableBase> &visitable){
      components.emplace_back(t,visitable);
    }

    template <class T,typename ... Component> void add_component(const std::shared_ptr<T> &component){
      add_component_for_type(get_type_index<T>(), component);
      dummy_function(add_component_for_type(get_type_index<Component>(), component) ...);
    }
    
    template <class T,typename ... Args> std::shared_ptr<T> create_component(Args ... args){
      auto component = std::make_shared<T>(args...);
      add_component(component);
      return component;
    }
    
    void remove_component_for_type(TypeIndex t){
      auto it = components.begin();
      while(it != components.end() && it->first != t) ++it;
      components.erase(it);
    }

    template <class T> void remove_component(){
      remove_component_for_type(get_type_index<T>());
    }

  };
  
 
  
  
}
