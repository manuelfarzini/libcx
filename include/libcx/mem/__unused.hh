/*

//--------------------------------------------
// Base

// using Align = std::align_val_t;
// using Nothrow = std::nothrow_t const&;
// onedef cons Nothrow No_Throw = std::nothrow;

onedef cons fn u64__mem_set(mutaptr data, u8 v, usize n) -> ErrorCode {
//  NOTE(manu)
//  u64 splat version, it's faster than gb.h inspired version but it's 
//  slower than libc ::memset
    if (data == null) unlike {
      return Invalid_Ptr;
    }
    if (n <= 0) unlike {
        return Invalid_Arg;
    }

    byteptr d  = byteptr(data);
    u64 v64 = cx_splat_8_64(v);

    for (; (iptr(d) & 7) != 0 && n > 0; n -= 1) {
        *d = v;
        d += 1;
    }

    u64ptr d64 = u64ptr(d);
    while (n >= 64) {
        d64[0] = v64;
        d64[1] = v64;
        d64[2] = v64;
        d64[3] = v64;
        d64[4] = v64;
        d64[5] = v64;
        d64[6] = v64;
        d64[7] = v64;
        d64 += 8;
        n -= 64;
    }

    d = byteptr(d64);
    while (n > 0) {
        *d = v;
        d += 1;
        n -= 1;
    }
    return none;
}

onedef cons fn gb__mem_set(mutaptr data, u8 v, usize n) -> ErrorCode {
//  NOTE(manu) 
//  gb.h mem_set inspired version
//  maybe I should use conditional compilation
//  probably can be faster reasoning on alignment

    if (data == null) unlike {
        return Invalid_Ptr;
    }
    if (n <= 0) unlike {
        return Invalid_Arg;
    }
    byteptr d = byteptr(data);

    d[0] = d[n - 1] = v;
    if (n < 3) unlike {
        return none;
    }
    d[1] = d[n - 2] = v;
    d[2] = d[n - 3] = v;
    if (n < 7) unlike {
        return none;
    }
    d[3] = d[n - 4] = v;
    if (n < 9) unlike {
        return none;
    }

    isize k = -iptr(d) & 3;
    d += k;
    n -= k;
    n &= -4;
    u32 v32 = cx_splat_8_32(v);

    *ptru32(d + 0) = v32;
    *ptru32(d + n - 4) = v32;
    if (n < 9) {
        return none;
    }
    *ptru32(d + 4) = v32;
    *ptru32(d + 8) = v32;
    *ptru32(d + n - 12) = v32;
    *ptru32(d + n - 8) = v32;
    if (n < 25) {
        return none;
    }
    *ptru32(d + 12) = v32;
    *ptru32(d + 16) = v32;
    *ptru32(d + 20) = v32;
    *ptru32(d + 24) = v32;
    *ptru32(d + n - 28) = v32;
    *ptru32(d + n - 24) = v32;
    *ptru32(d + n - 20) = v32;
    *ptru32(d + n - 16) = v32;

    k = 24 + (uptr(d) & 4);
    d += k;
    n -= k;

    {
        u64 v64 = cx_splat_32_64(v32);
          while (n > 31) {
            *u64ptr(d + 0) = v64;
            *u64ptr(d + 8) = v64;
            *u64ptr(d + 16) = v64;
            *u64ptr(d + 24) = v64;

            n -= 32;
            d += 32;
        }
    }

    return none;
}

onedef cons fn odin__mem_zero_condition(mutaptr data, isize size) -> void
//  NOTE(manu)
//  odin like version
//  it's slower compared to the gb.h memset inspired one
{
    if (data == null || size <= 0) {
        return;
    }

    auto bytes_num = uptr(size);
    auto words_num = bytes_num / Ptr_Size;

    auto words_beg = viewuptr(data);
    auto words_end = viewuptr(data) + words_num;

    auto bytes_beg = byteptr(words_end);
    auto bytes_end = byteptr(data) + bytes_num;

    for (; words_beg < words_end; words_beg++) {
        if (*words_beg != 0) {
            *words_beg = 0;
        }
    }

    for (; bytes_beg < bytes_end; bytes_beg += 1) {
        if (*bytes_beg != 0) {
            *bytes_beg = 0;
        }
    }
}

*/
