
#include <lars/visitor.h>
#include <memory>

namespace lars {
  
  class Any {
  private:
    std::unique_ptr<VisitableBase> data;
  public:
    
    Any():data(std::make_unique<EmptyVisitable>()){}
    
    template <class T, typename ... Bases> void set(T &&){
      // data.reset(new ScalarDataVisitable<T, InheritanceList<OrderedType<T, 0>, OrderedType<Bases, 0> ...>>(t));
    }
    
    template <class T> T &get(){
      return visitor_cast<T>(data);
    }
    
    template <class T> const T &get()const{
      return visitor_cast<const T>(data);
    }
    
  };
  
}
