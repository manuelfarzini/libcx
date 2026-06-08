/** @file conf/prelude.hh **/

#ifndef CX_CONF_PRELUDE_HH
#define CX_CONF_PRELUDE_HH

#include "libcx/conf/hal.hh"
#include "libcx/conf/macro.hh"

namespace cx {

template<bool B> predicate kpred = B;

template<usize bit>
onedef cons usize BIT = 1ull << bit;

using ErrorCode = i32;
onedef cons i32 Error_None = 0;
onedef cons i32 Operation_Fail = 1;
onedef cons i32 Invalid_Ptr = 2;
onedef cons i32 Invalid_Arg = 3;
onedef cons i32 Alloc_Exhausted = 4;
onedef cons i32 Alloc_Invalid_Mode = 5;

}

#endif  // CX_CONF_PRELUDE_HH
