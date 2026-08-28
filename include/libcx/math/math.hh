/** @file libcx/math/math.hh **/

#ifndef CX_MATH_MATH_HH
#define CX_MATH_MATH_HH

#include "libcx/config.hh"
#include "libcx/uti/typeseq.hh"

namespace cx {
inline namespace math {

/** Computes the maximum value. **/
template<typename Head, typename... Rest>
inln cons fn max(Head head, Rest... rest) noexce -> Head where(
    va_size_of(Rest) > 0 && va_is_homo<Head, Rest...> && size_of(Head) <= 8
)
{
    Head max = head;
    ((max = max < rest ? rest : max), ...);
    return max;
}

/** Computes the maximum value. **/
template<typename Head, typename... Rest>
inln cons fn max(Head& head, Rest&... rest) noexce -> Head&
    where (va_size_of(Rest) > 0 && va_is_homo<Head, Rest...> && size_of(Head) > 8)
{
    Head const* max = &head;
    ((max = *max < rest ? &rest : max), ...);
    return *max;
}

}       // namespace math
}       // namespace cx
#endif  // CX_MATH_MATH_HH
