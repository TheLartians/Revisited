#pragma once

#include <lars/type_index.h>
#include <lars/visitor.h>

namespace lars{
  
  template <typename ...> class VisitableTypeIndex;
  
  template<> class VisitableTypeIndex<>:public Visitable<VisitableTypeIndex<>>{};
  
  template <class I> class VisitableTypeIndex<I>:public DVisitable<I,VisitableTypeIndex<>>{
    static constexpr TypeIndex value = get_type_index<I>();
  };
  
  
}
