/** 
    @file rnd/xoroshiro128plus.cc
    SIMD-friendly adaptation of the xoroshiro128+ PRG by Vigna and Blackman.
    tailored for fast generation of 32-bit audio noise.
**/

#ifndef CX_RND_XOROSHIRO128P_HH
#define CX_RND_XOROSHIRO128P_HH

#include "libcx/uti/utilities.hh"

glob cons f32 F32_NORM_TO_01 = 2.0f/4294967296.0f;

namespace cx::rnd {

/// A xorshift based pseudo random number generator.
/// It encapsulates a variation of the xoroshiro128+ algorithm
/// by Vigna and Blackman optimized for SIMD-based generation
/// of 32-bit noise. It has a 128b `state` and a `simd_buffer`
struct Xoroshiro128p {
    u64 state[2];
    alignas(16) f32 simd_buff[4];
};

/// Rotates a 64-bit unsigned integer `y` left by `k` bits.
/// Returns: the rotated value.
inln cons fn rotl(u64 const y, int k) noexce -> u64
{
    return (y << k) | (y >> (64 - k));
}

/// Splitmix64 algorithm.
/// Generates an u64 pseudorandom number from the input seed `y`
/// that will be mutated. Initializes the `Xoroshiro`.
/// Returns: the u64 generated pseudorandom number.
inln cons fn splitmix64(u64& y) noexce -> u64
{
    u64 z = (y += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

/// Seeds the internal state of `xoro` using the splitmix64
/// algorithm with the input seed `y`.
inln cons fn seed(Xoroshiro128p& xoro, u64 y) noexce -> void
{
    xoro.state[0] = splitmix64(y);
    xoro.state[1] = splitmix64(y);
}

/// Generates the next u64 pseudorandom value from `xoro`
/// Returns: the u64 generated pseudorandom number
inln cons fn next(Xoroshiro128p& xoro) noexce -> u64
{
    const u64 s0 = xoro.state[0];
    u64 s1 = xoro.state[1];
    const u64 res = s0 + s1;
    s1 ^= s0;
    xoro.state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
    xoro.state[1] = rotl(s1, 37);
    return res;
}

/// Generates a single f32 ∈ [-1, 1) from a `xoro`.
/// Returns: the generated f32.
inln cons fn generate_one(Xoroshiro128p& xoro) noexce -> f32
{
    return 2.0f * f32(next(xoro)>>32) * (1.0f/f32(1ull<<32)) - 1.0f;
}

/// Generates four f32 numbers ∈ [-1, 1) from a `xoro` using
/// SIMD instructions. Writes them in the internal `simd_buffer`
/// and a pointer to it is returned.
/// Returns: a pointer to the internal `simd buffer`.
inln cons fn generate_four(Xoroshiro128p& xoro) noexce -> f32*
{
    #if CX_SSE2
        __m128i vi = _mm_set_epi32(
            int(next(xoro)),
            int(next(xoro) >> 32),
            int(xoro.state[1]),
            int(xoro.state[0] >> 32)
        );

        __m128 vf = _mm_cvtepi32_ps(vi);
        _mm_storeu_ps(
            xoro.simd_buff,
            _mm_sub_ps(
                _mm_mul_ps(vf, _mm_set1_ps(F32_NORM_TO_01)),
                _mm_set1_ps(1.0f))
        );

        return xoro.simd_buff;

    #elif CX_NEON
    float32x4_t vf = vcvtq_f32_u32(
        uint32x4_t{
            u32(xoro.state[0] >> 32),
            u32(xoro.state[1]),
            u32(next(xoro) >> 32),
            u32(next(xoro))
        });
    vf = vsubq_f32(vmulq_f32(vf, vdupq_n_f32(F32_NORM_TO_01)), vdupq_n_f32(1.0f));
    vst1q_f32(xoro.simd_buff, vf);
    return xoro.simd_buff;

    #else
        xoro.simd_buff[0] = f32(next(xoro) >> 32) * (F32_NORM_TO_01) - 1.0f;
        xoro.simd_buff[1] = f32(next(xoro)) * (F32_NORM_TO_01) - 1.0f;
        xoro.simd_buff[2] = f32(xoro.state[0] >> 32) * (F32_NORM_TO_01) - 1.0f;
        xoro.simd_buff[3] = f32(xoro.state[1]) * (F32_NORM_TO_01) - 1.0f;
        return xoro.simd_buff;

    #endif
}

#undef  F32_NORM_TO_01
}       // namespace cx::rnd
#endif  // CX_RND_XOROSHIRO128P_HH
