#ifndef CX_UTI_DISTINCT_HH
#define CX_UTI_DISTINCT_HH

#include "libcx/conf/macro.hh"

template<typename T, auto f>
struct _Distinct {
    using Type = T;
    T value;
    onedef _Distinct() = default;
    onedef _Distinct(T v) : value(v) {}
};

#ifndef cx_distinct
    #define cx_distinct(T) _Distinct<T, [](){}>
#endif

#endif  // CX_UTI_DISTINCT_HH
