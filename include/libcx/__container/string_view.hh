/** **/

#include "libcx/conf.hh"
#include "libcx/uti/iterator.hh"

onedef cons fn cx_has_zero_byte_u64(u64 x) -> b32 {
    // Hacker's Delight: detect any zero byte
    return ((x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL) != 0;
}

inln onedef cons fn cx_first_zero_byte_u64(u64 x) -> u32 {
    u64 m = (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;

    #if defined(CX_COMPILER_GCC) || defined(CX_COMPILER_CLANG)
        return u32(__builtin_ctzll(m) >> 3);

    #elif defined(CX_COMPILER_MSVC) && defined(CX_ARCH_64)
        u32 idx;
        _BitScanForward64(&idx, m);
        return u32(idx >> 3);

    #else
        u8 const* b = ptrcu8(&x);
        for (u32 i = 0; i < 8; ++i) 
            if (b[i] == 0) 
                return i;
        return 8;
    #endif
}

inln onedef cons fn cx_strlen_u8(u8 const* s) -> isize {
    const u8* p = s;

    // align to 8 (optional; helps some CPUs)
    while ((uptr(p) & 7u) != 0u) {
        if (*p == 0) return (isize)(p - s);
        ++p;
    }

    for (;;) {
        u64 x;
        // memcpy avoids UB from strict-aliasing and unaligned loads
        ::memcpy(&x, p, sizeof(x));

        if (cx_has_zero_byte_u64(x)) {
            return isize(p - s) + cx_first_zero_byte_u64(x);
        }
        p += 8;
    }
}

inln onedef cons fn cx_strlen(char const* txt) noexce -> isize
{
    return isize(::strlen(txt));
}

/**
    @pre
    - `txt` must be null-terminated
**/
inln onedef cons fn cx_strlen(u8 const* txt) noexce -> isize
{
    return isize(::strlen(ptrcch8(txt)));
}

#ifndef CX_STRING_HH
#define CX_STRING_HH

namespace cx {

struct vstring {
    CX_DEFINE_MEMBER_TYPES(u8, isize);
    using Self = vstring;

    u8* ptr;
    Size len;

    inln cons fn operator[](Size const idx) noexce -> u8 const& { return ptr[idx]; }

    inln cons fn beg() noexce -> Iter { return ptr; }
    inln cons fn end() noexce -> Iter { return ptr + len; }

    inln cons fn beg() const noexce -> Kter { return ptr; }
    inln cons fn end() const noexce -> Kter { return ptr + len; }
};

template<typename T>
inln onedef cons fn slice(T const& src) noexce -> vstring;

template<typename T>
inln onedef cons fn slice(T const* src) noexce -> vstring;

template<typename T, typename S>
inln onedef cons fn slice(T const* src, S len = -1) noexce -> vstring;

template<typename T>
predicate is_string_like = requires(T t) { []{ cx::slice(t); }(); };

template<> predicate is_string_like<vstring> = true;

struct StringIter {
    using Size = isize;
    using Self = StringIter;

    vstring const& str;
    isize pos = 0;
};

template<>
inln onedef cons fn slice(u8 const* txt, isize len) noexce -> vstring
{
    if (len < 0) unlike {
        len = cx_strlen(txt);
    }
    return {byteptr(txt), len};
}

template<>
inln onedef cons fn slice(char const* txt) noexce -> vstring
{
    return slice(ptrcu8(txt), cx_strlen(txt));
}

}       // namespace cx
#endif  // CX_STRING_HH
