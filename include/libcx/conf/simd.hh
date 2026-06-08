#ifndef CX_CONF_SIMD_HH
#define CX_CONF_SIMD_HH

#include "libcx/conf/hal.hh"

namespace cx {

enum struct dtype : u32 { f32, f64 };
template<dtype d, usize n> struct _simd;

#if CX_NEON

template<> struct _simd<dtype::f32, 4> { using Type = float32x4_t; };
template<> struct _simd<dtype::f64, 2> { using Type = float64x2_t; };
template<> struct _simd<dtype::f64, 4> { using Type = float64x2_t; };

#elif CX_SSE2

template<> struct _simd<dtype::f32, 4> { using Type = __m128; };
template<> struct _simd<dtype::f64, 2> { using Type = __m128d; };
template<> struct _simd<dtype::f64, 4> { using Type = __m128d; };

#endif

}       // namespace cx
#endif  // CX_CONF_SIMD_HH
