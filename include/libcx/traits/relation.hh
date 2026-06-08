/** @file libcx/traits/relation.hh **/

#ifndef CX_TRAITS_RELATION_HH
#define CX_TRAITS_RELATION_HH

#include "libcx/uti/declval.hh"

namespace cx {

// Comparison

template<typename T, typename U> predicate ___same_as = false;
template<typename T> predicate ___same_as<T, T> = true;
template<typename T, typename U> predicate same_as = ___same_as<T, U>;

template<typename T, typename... Ts> predicate one_of = (___same_as<T, Ts> || ...);

// NOTE(manu) the taparam `T` must be a plain type (not cvref) in order to work as intended
template<typename T, typename U> predicate same_or_cv = same_as<T, rm_cv<U>>;
template<typename T, typename U> predicate same_or_ref = same_as<T, rm_ref<U>>;
template<typename T, typename U> predicate same_or_cvref = same_as<T, rm_cvref<U>>;

template<typename T, typename U> predicate is_convertible = requires { static_cast<U>(declv<T>()); };

// Conversion

template<typename U> void accept__implicit_convertible(U);

template<typename T, typename U>
predicate is_implicit_convertible = requires { accept__implicit_convertible<U>(declv<T>()); };

template<typename T, typename U>
using ternary_res = declt(false ? declv<T>() : declv<U>());

}       // namespace cx
#endif  // CX_TRAITS_RELATION_HH
