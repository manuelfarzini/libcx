/** @file libcx/uti/rvalue.hh **/

#ifndef CX_UTI_RVALUE_HH
#define CX_UTI_RVALUE_HH

#include "libcx/conf/macro.hh"
#include "libcx/uti/sfinae.hh"
#include "libcx/uti/decay.hh"
#include "libcx/uti/common_ref.hh"

namespace cx {
inline namespace uti {

// Rvalue pitfalls solutions
// We have 3 mitigation tricks

// 1.
// Deleting the rvalue accessors.
// struct Example {
//   sint32 a;
//   sint32&& get_a() && = delete;
// };

// 2.
// This trait + macro system ensures safe reference binding when
// forwarding expressions that may be lvalues or rvalues.
// In particular, it prevents dangling references when binding an
// rvalue to a local variable that may be returned later.
// If the input is an lvalue, the reference is preserved; if it's
// an rvalue, it is decayed to a value.
// This avoids ub caused by returning references to temporaries.
// Example of usage:
// ```
// decltype(auto) example() {
//   cx_auto_ref(p, get_some_person());
//   ... do something with p ...
//   return person;
// }
// ```
// Example of usage:
// ```
// struct A {};
// struct C {
//   A get_a() const&;
// };
// template<typename T, typename U>
// decltype(auto) min(T&& x, U&& y) {  
//   return x < y ? uti::forward<T>(x) : uti::forward<U>(y);
// }
// bool operator<(A const&, A const&);
// void test() {
//   C c1, c2;
//   // auto const& a = min(c1.get_a(), c2.get_a()); dangling
//   cx_auto_ref(a, min(c1.get_a(), c2.get_a()));
// }
// ```
// https://www.youtube.com/watch?v=9V195P3Tv_8 18:22
template<typename T> struct ___decay_rvalue {};
template<typename T> struct ___decay_rvalue<T&> { using type = T&; };
template<typename T> struct ___decay_rvalue<T&&> { using type = decay<T>; };
template<typename T> using decay_rvalue = typename ___decay_rvalue<T>::type;
#define cx_auto_ref(var, ...) decay_rvalue<declt((__VA_ARGS__))&&> var = (__VA_ARGS__);

// 3.
// A custom version of the common reference trait
// The `std::common_reference_t` trait tends to collapse mixed cases
// like `int&` and `int&&` into `int&`, losing rvalue semantics.
// The libcx `uti::common_ref` trait behaves in a similar way.
// We provide another version, the `cx::uti::common_ref_rval` trait.
// It extends the behavior to preserve rvalue category:
// - if `uti::common_ref` would be an lvalue reference but any input
//   type is an rvalue reference, then the result becomes an rvalue
//   reference to the base type.
// - otherwise, it yields the same as `uti::common_ref`.
// It's useful in forwarding contexts to avoid accidental copies when
// combining lvalues and temporaries.
template<typename... Ts> 
struct ___common_ref_rval {
  using Old = common_ref<Ts...>;
  using Type = condition<is_lref<Old> && (is_rref<Ts> || ...), rm_ref<Old>&&, Old>;
};
template<typename... Ts> using common_ref_rval = ___common_ref_rval<Ts...>::Type;
// Example of usage:
// template<typename T, typename U>
// common_ref_prefer_rvalue<T, U> help__test(T&& a, U&& b) {
//   return cx::uti::forward<U>(b);
// }
// void test() {
//   struct Resource {} x;
//   auto v2 = help__test(x, Resource{});
// }
// Binds to `const&&` if there is an rvalue.
// Prevents an immediate copy.
#define cx_condition_ref(b, l, r)                                       \
  (b ? static_cast<common_ref<declt((l)), declt((r))>>(l)  \
     : static_cast<common_ref<declt((l)), declt((r))>>(r))
// Example of usage:
// struct A {};
// struct B {
//   A a;
//   A const& get_al() { return a; }
//   A&& get_ar() { return cx::take(a); }
// };
//
// int something() { return 42; }
// void Example() {
//   auto x = something();
//   B b;
//   auto y = cx_condition_ref(x, b.get_al(), b.get_ar());
// }

}       // namespace cx
}       // namespace uti
#endif  // CX_UTI_RVALUE_HH
