/** @file libcx/traits/logic.hh **/

#ifndef CX_TRAITS_LOGIC_HH
#define CX_TRAITS_LOGIC_HH

#include "libcx/conf/macro.hh"
#include "libcx/traits/relation.hh"
#include "libcx/uti/ownership.hh"

namespace cx {
inline namespace uti {

// Operators

template<bool... Bs> predicate bvariand = (Bs && ...);
template<typename... Ts> predicate tvariand = (Ts::value && ...);
template<bool... Bs> predicate bvarior = (Bs || ...);
template<typename... Ts> predicate tvarior = (Ts::value || ...);

// Assertable

template<typename T> concept ___assertble = is_convertible<T, bool>;

template<typename T>
predicate is_assertble = ___assertble<T> && requires(T&& t) { { !uti::forward<T>(t) } -> ___assertble; };

template<typename T> concept Assertble = is_assertble<T>;

}       // namespace uti
}       // namespace cx
#endif  // CX_TRAITS_LOGIC_HH
