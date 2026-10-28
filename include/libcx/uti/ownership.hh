/** @file libcx/uti/ownership.hh **/

#ifndef CX_UTI_OWNERSHIP_HH
#define CX_UTI_OWNERSHIP_HH

#include "libcx/conf/prelude.hh"
#include "libcx/traits/qualifier.hh"

namespace cx {
inline namespace uti {

template<typename T>
nodisc inln cons fn forward(rm_ref<T>& x) noexce -> T&&
{
  return static_cast<T&&>(x);
}

template<typename T> where (!is_lref<T>)
nodisc inln cons fn forward(rm_ref<T>&& x) noexce -> T&&
{
  return static_cast<T&&>(x);
}

template<typename T>
nodisc inln cons fn take(T&& x) noexce -> rm_ref<T>&&
{
  return static_cast<rm_ref<T>&&>(x);
}

// only xvalues/prvalues are passed as T&&
template<typename T>
nodisc inln cons fn pass(T&& x) noexce -> T&&
{
  return static_cast<rm_ref<T>&&>((x));
}

}       // namespace uti
}       // namespace cx
#endif  // CX_UTI_OWNERSHIP_HH
