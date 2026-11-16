/** @file conf/prelude.hh **/

#ifndef CX_CONF_PRELUDE_HH
#define CX_CONF_PRELUDE_HH

#include "libcx/conf/hal.hh"
#include "libcx/conf/macro.hh"

namespace cx {

template<usize bit>
onedef cons usize BIT = 1ull << bit;

using ErrorCode = i32;
onedef cons ErrorCode Error_None = 0;
onedef cons ErrorCode Operation_Fail = 1;
onedef cons ErrorCode Invalid_Ptr = 2;
onedef cons ErrorCode Invalid_Arg = 3;
onedef cons ErrorCode Alloc_Exhausted = 4;
onedef cons ErrorCode Alloc_Invalid_Mode = 5;

}

#endif  // CX_CONF_PRELUDE_HH
