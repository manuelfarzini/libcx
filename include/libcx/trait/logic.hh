/** @file libcx/trait/logic.hh **/

#ifndef CX_TRAIT_LOGIC_HH
#define CX_TRAIT_LOGIC_HH

#include "libcx/conf/macro.hh"
#include "libcx/trait/relation.hh"

namespace cx {
inline namespace uti {

// Operators

template<bool... Bs> proposition bvariand = (Bs && ...);
template<typename... Ts> proposition tvariand = (Ts::value && ...);
template<bool... Bs> proposition bvarior = (Bs || ...);
template<typename... Ts> proposition tvarior = (Ts::value || ...);

// Assertable

template<typename T> concept ___assertble = is_convertible<T, bool>;

template<typename T> proposition is_assertble = ___assertble<T> && requires(T&& t) {
    { !static_cast<T&&>(t) } -> ___assertble;
};

template<typename T> concept Assertble = is_assertble<T>;

}       // namespace uti
}       // namespace cx
#endif  // CX_TRAIT_LOGIC_HH
