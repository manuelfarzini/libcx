#ifndef CX_UTI_DISTINCT_HH
#define CX_UTI_DISTINCT_HH

#include "libcx/conf/macro.hh"

template<typename T, auto f>
struct Distinct {
    using Type = T;
    T value;
    onedef Distinct() = default;
    onedef Distinct(T v) : value(v) {}
};

#ifndef cx_distinct
    #define cx_distinct(T) Distinct<T, [](){}>
#endif

#endif  // CX_UTI_DISTINCT_HH
