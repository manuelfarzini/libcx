/** @file libcx/uti/utilities.hh **/

#ifndef CX_UTI_UTILITIES_HH
#define CX_UTI_UTILITIES_HH

#include "libcx/uti/ownership.hh"
#include "libcx/uti/defer.hh"
#include "libcx/uti/iterator.hh"
#include "libcx/uti/sfinae.hh"
#include "libcx/uti/members.hh"
#include "libcx/uti/container.hh"
#include "libcx/uti/compare.hh"
#include "libcx/uti/rvalue.hh"
#include "libcx/uti/typeseq.hh"

namespace cx {
inline namespace uti {

/// Returns `true` if `x` is a power of two.
inln cons fn is_power_of_two(isize const x) noexce -> b32
{
    if (x <= 0) {
        return false;
    }
    return !(x & (x - 1));
}

/// Holds if `x` is a power of two.
template<isize x>
predicate is_pow2 = is_power_of_two(x);

}       // namespace uti
}       // namespace cx
#endif  // CX_UTI_UTILITIES_HH
