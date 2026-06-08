/** @file libcx/uti/decay.hh **/

#ifndef CX_UTI_DECAY_HH
#define CX_UTI_DECAY_HH

#include "libcx/conf/macro.hh"
#include "libcx/traits/types.hh"
#include "libcx/uti/sfinae.hh"

namespace cx {
inline namespace uti {

template<typename T>
struct _decay
{
    using U = rm_ref<T>;
    using Type = 
        condition<is_raw_array<U>, typename add_ptr<rm_extent<U>>::Type,
        condition<is_func<U>,      typename add_ptr<U>::Type,
        /** else **/               rm_cv<U>
    >>;
};

template<typename T> using decay = _decay<T>::Type;

}      // namespace uti
}      // namespace cx
#endif // CX_UTI_DECAY_HH
