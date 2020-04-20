#pragma once

#include <functional>
#include <type_traits>

namespace revisited {
  
  // from https://stackoverflow.com/questions/11893141/inferring-the-call-signature-of-a-lambda-or-arbitrary-callable-for-make-functio
  template<typename T> struct remove_class { };
  template<typename C, typename R, typename... A>
  struct remove_class<R(C::*)(A...)> { using type = R(A...); };
  template<typename C, typename R, typename... A>
  struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
  template<typename C, typename R, typename... A>
  struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
  template<typename C, typename R, typename... A>
  struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };
  
  template<typename T>
  struct get_signature_impl { using type = typename remove_class<
    decltype(&std::remove_reference<T>::type::operator())>::type; };
  template<typename R, typename... A>
  struct get_signature_impl<R(A...)> { using type = R(A...); };
  template<typename R, typename... A>
  struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
  template<typename R, typename... A>
  struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
  template<typename T> using get_signature = typename get_signature_impl<T>::type;
  
  template<typename F> using make_function_type = std::function<get_signature<F>>;
  template<typename F> make_function_type<F> make_function(F &&f) {
    return make_function_type<F>(std::forward<F>(f)); }
  
}