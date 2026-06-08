/** @file conf/type.hh **/

#ifndef CX_CONF_TYPES_HH
#define CX_CONF_TYPES_HH

#include <initializer_list>

#include "libcx/conf/hal.hh"
#include "libcx/conf/macro.hh"
#include "libcx/uti/tuple.hh"

namespace cx {

using cstring = char const*;

template<typename T>     using Void = void;
template<typename T>     using initls = std::initializer_list<T>;
template<typename... Ts> using Res = Tuple<Ts...>;

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
#endif  // CX_CONF_TYPES_HH
