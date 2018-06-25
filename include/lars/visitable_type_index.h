#pragma once

#include <lars/type_index.h>
#include <lars/visitor.h>

namespace lars{
  
  template <class I> class VisitableTypeIndex:public Visitable<I>{
    static const TypeIndex value = get_type_index<I>();
  };
  
}
