/** @file libcx/concept/multi.hh **/

#ifndef CX_CONCEPTS_MULTI_HH
#define CX_CONCEPTS_MULTI_HH

#include "libcx/concept/type.hh"
#include "libcx/traits/relation.hh"
#include "libcx/traits/types.hh"

namespace cx { 
inline namespace uti {

template<typename U, typename... Ts> 
concept PlainArithmeticAmong = SomePlainArithmetic<U> && one_of<U, Ts...>;

template<typename U, typename... Ts>
concept PlainRawPointerAmong = SomePlainRawPointer<U> && one_of<U, Ts...>;

}       // namespace uti
}       // namespace cx
#endif  // CX_CONCEPTS_MULTI_HH
