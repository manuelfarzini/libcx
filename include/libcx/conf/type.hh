/** @file conf/type.hh **/

#ifndef CX_CONF_TYPE_HH
#define CX_CONF_TYPE_HH

#include <initializer_list>

#include "libcx/conf/hal.hh"
#include "libcx/conf/macro.hh"

namespace cx {

using cstring = char const*;

template<typename T>     using Void = void;
template<typename T>     using initls = std::initializer_list<T>;

struct NoneType {};
onedef cons NoneType None;

struct ZeroType {
    cons operator isize() const noexce { return 0; }
    cons operator void*() const noexce { return nullptr; }
    cons operator decltype(nullptr)() const noexce { return nullptr; }
};
onedef cons ZeroType null;
onedef cons ZeroType none;


}       // namespace cx
#endif  // CX_CONF_TYPE_HH
