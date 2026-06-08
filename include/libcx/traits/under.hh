#ifndef CX_TRAITS_UNDER_HH
#define CX_TRAITS_UNDER_HH
#include <libcx/conf/hal.hh>

template<isize bytes> struct ___BytesToSint {};
template<> struct ___BytesToSint<1> { using T = i8; };
template<> struct ___BytesToSint<2> { using T = i16; };
template<> struct ___BytesToSint<4> { using T = i32; };
template<> struct ___BytesToSint<8> { using T = i64; };

template<usize bytes>
using BytesToSint = ___BytesToSint<bytes>::T;

template<isize bytes> struct ___BytesToUint {};
template <> struct ___BytesToUint<1> { using T = u8; };
template <> struct ___BytesToUint<2> { using T = u16; };
template <> struct ___BytesToUint<4> { using T = u32; };
template <> struct ___BytesToUint<8> { using T = u64; };

template<usize bytes>
using BytesToUint = ___BytesToUint<bytes>::T;

template<typename T>
using UnderSint = BytesToSint<sizeof(T)>;

template<typename T>
using UnderUint = BytesToUint<sizeof(T)>;

#endif  // CX_TRAITS_UNDER_HH
