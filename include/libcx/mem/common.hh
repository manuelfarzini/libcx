/** @file libcx/mem/common.hh **/

#ifndef LIBCX_MEM_COMMON_HH
#define LIBCX_MEM_COMMON_HH

#include "libcx/conf/simd.hh"
#include "libcx/trait.hh"
#include "libcx/concept.hh"
#include "libcx/uti/ownership.hh"
#include "libcx/uti/tuple.hh"

CX_HIDE_FROM_ABI nodisc cons fn operator new(usize, mutaptr __p) noexce -> mutaptr { return __p; }
CX_HIDE_FROM_ABI        cons fn operator delete(mutaptr, mutaptr)  noexce -> void  {             }

namespace cx {
inline namespace mem {

onedef cons isize PTR_SIZE  = size_of(mutaptr);
onedef cons isize PTR_ALIGN = align_of(mutaptr);
onedef cons isize MAX_SIZE  = size_of(max_align_t);
onedef cons isize MAX_ALIGN = align_of(max_align_t);
onedef cons isize DEF_ALIGN = 2 * MAX_ALIGN;

static_assert(MAX_ALIGN % PTR_ALIGN == 0, "`MAX_ALIGN` must be a multiple of `ptr_align`");
static_assert((MAX_ALIGN & (MAX_ALIGN - 1)) == 0, "`MAX_ALIGN` must be a power of 2");
static_assert(DEF_ALIGN == 8  || DEF_ALIGN == 16, "`DEF_ALIGN` should be 8 or 16");

/** Predicate that is `true` if `Tp` requires more alignment than `max_align_t`. **/
template<typename Tp>
predicate is_over_aligned =  align_of(Tp) > align_of(max_align_t);

/**
    Copies `size` bytes from `src` to `dst`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    - `size`: the number of bytes to copy.
    @req
    - `dst` refers to storage valid for `size` bytes.
    - `src` refers to storage valid for `size` bytes.
    - Source and destination ranges do not overlap.
**/
inln cons fn
mem_copy(mutaptr cx_restrict dst, readptr cx_restrict src, isize size) noexce -> void
{
    cx_assume(byteptr(dst) + size <= src || byteptr(src) + size <= dst);
    ::memcpy(dst, src, size);
}

/**
    Copies `num` elements of type `T` from `src` to `dst`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    - `num`: the number of elements to copy.
    @req
    - `dst` refers to storage valid for `num` elements of type `T`.
    - `src` refers to storage valid for `num` elements of type `T`.
    - Source and destination ranges do not overlap.
**/
template<typename T>
inln cons fn mem_copy(T* cx_restrict dst, T const* cx_restrict src, isize num) noexce -> void
    where (not is_void<T>)
{
    cx_assume(
        uptr(dst) + num * size_of(T) <= uptr(src) || uptr(src) + num * size_of(T) <= uptr(dst)
    );
    mem_copy(mutaptr(dst), mutaptr(src), num * size_of(T));
}

/** 
    Copies `size` bytes from `src` to `dst`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    @pre
    - `src` and `dst` have `size` bytes.
**/
inln cons fn mem_move(mutaptr dst, readptr src, isize size) -> void
{
    ::memmove(dst, src, size);
}

/** 
    Copies `num` elements of type `T` from `src` to `dst`.
    @arg
    - `dst`: the destination pointer
    - `src`: the source pointer
    @pre
    - `src` and `dst` have `num` elements
**/
template<typename T>
inln cons fn mem_move(T* dst, T const* src, isize num) noexce -> void where (not is_void<T>)
{
    mem_move(mutaptr(dst), mutaptr(src), num * size_of(T));
}

/** 
    Takes `num` elements of type `T` from `src` to `dst`
    If `T` is not move assignable, then behaves like `memcopy`.
    @arg
    - `dst`: the destination pointer
    - `src`: the source pointer
    @pre
    - `src` and `dst` have `num` elements
**/
template<CpOrMvAsble T>
cons fn mem_take(T* dst, T const* src, isize num) noexce -> void where (not is_void<T>)
{
    for (isize i = 0; i < num; i++) {
        dst[i] = take(src[i]);
    }
}

/** 
    Sets `n` bytes of memory at `data` to `v`.
    @arg
    - `data`: pointer to the memory to be set
    - `v`: value to set the memory to
    - `n`: size of the memory to be set [Byte]
    @pre
    - `data` has at least `n` bytes.
**/
cons fn mem_set(mutaptr data, u8 val, isize size) -> ErrorCode
{
    // NOTE(manu)
    // - libc wrapper, actually is the fastest
    // - found that is the fastest on my m1 macbook pro
    //   since it is implemented in arm64 asm
    // - maybe on other platoforms I should use another version

    // #if defined(CX_SYSTEM_OSX)
        mutaptr res = ::memset(data, val, size);
        if (!res) {
            return Operation_Fail;
        }
    // #endif
    return none;
}

/** 
    Zeroes `size` bytes of memory at `data`.
    @arg
    - `data`: pointer to the memory to be zeroed
    - `size`: size of the memory to be zeroed [Byte]
    @ret
    - `ErrorCode`
    @pre
    - `data` has at least `size` bytes.
    - `data` is not null
**/
cons fn mem_zero(mutaptr data, isize size) -> ErrorCode
{
    return mem_set(data, 0, size);
}

/** 
    Zeroes `size` bytes of memory at `data`.
    @arg
    - `data`: pointer to the memory to be zeroed
    - `size`: size of the memory to be zeroed [Byte]
    @ret
    - The generated `ErrorCode` if any, `none` otherwise
    @pre
    - `data` has at least `size` bytes.
    @nota
    - intended to be used when dealing with virtual memory as it
      would be already mapped to a 0 page
**/
cons fn mem_zero_condition(mutaptr data, isize size) -> ErrorCode
{
    // NOTE(manu)
    // gb.h memset inspired version
    // it's faster compared to the odin like one and it's much faster than
    // libc ::memset with 0s if the memory is already mapped to a 0 page
    // probably I can improve this even more under certain assumptions,
    // like using a u64 splat directly
    // TODO:(manu) what about using ::memset call conditionally?
    if (data == null) unlike {
        return Invalid_Ptr;
    }
    if (size <= 0)  unlike {
        return Invalid_Arg;
    }

    #define zero_if_not(x)   \
        if (x != 0) unlike { \
                x = 0;       \
            }
    #define ret_if_less(a, b) \
        if (a < b) {          \
            return none;      \
        }

    byteptr d = byteptr(data);

    zero_if_not(d[0]);
    zero_if_not(d[size - 1]);
    ret_if_less(size, 3);

    zero_if_not(d[1]);
    zero_if_not(d[size - 2]);
    zero_if_not(d[2]);
    zero_if_not(d[size - 3]);
    ret_if_less(size, 7);

    zero_if_not(d[3]);
    zero_if_not(d[size - 4]);
    ret_if_less(size, 9);

    iptr k = -iptr(d) & 3;
    for (isize i = 0; i < k; i += 1) {
        if (d[i] != 0) {
            d[i] = 0;
        }
    }

    d += k;
    size -= k;
    size &= -4;
    auto d64 = cast(u64*,d);
    for (; size >= 32 ;) {
        if ((d64[0] | d64[1] | d64[2] | d64[3]) != 0) {
            d64[0] = 0;
            d64[1] = 0;
            d64[2] = 0;
            d64[3] = 0;
        }

        d += 32;
        size -= 32;
    }

    for (; size >= 4 ;) {
        zero_if_not(*cast(u32*, d));
        d += 4;
        size -= 4;
    }

    #undef zero_if_not
    #undef ret_if_less
    return none;
}

/** 
    Zero intializes `num` contiguous `T` objects at `ptr`.
    @arg
    - `ptr`: the pointer to the first element
    - `num`: the number of elements
    @ret
    - `[null, err]` if `ptr == null` or `num` == `0`;
    - `[ptr, none]` otherwise.
    @pre
    - Sufficient, properly aligned storage at `ptr`.
**/
template<ZeroInitble T>
nodisc inln cons fn init_type(mutaptr ptr, isize num) noexce -> ErrorCode
{
    if (num == 0) {
        return Invalid_Arg;
    }
    if (ptr == null) {
        return Invalid_Ptr;
    }

    for (isize i = 0; i < num; i++) {
        ::new (cast(T*, ptr) + i) T();
    }

    return none;
}

/**
    Initializes `num` contiguous `T` objects at `ptr` using `args` for each.
    @arg
    - `ptr`: the pointer to the first element;
    - `num`: the number of elements;
    - `args`: the arguments to the constructor.
    @ret
    - `[null, err]` if `ptr == null` or `num` == `0`;
    - `[ptr, none]` otherwise.
    @pre
    - Sufficient, properly aligned storage at `ptr`.
    @note
    - `args` are reused `num` times so they are taken by `const&` not forwarded.
**/
template<ZeroInitble T, typename... Args>
nodisc cons fn init_va(
    mutaptr ptr, isize num, Args const&... args
) noexce -> Res<T*, ErrorCode>
    where is_ctble<T, Args const&...>
{
    if (num == 0) {
        return {null, Invalid_Arg};
    }
    if (ptr == null) {
        return {null, Invalid_Ptr};
    }
    for (isize i = 0; i < num; i++) {
        ::new (cast(T*, ptr) + i) T{args...};
    }
    return {cast(T*, ptr), none};
}

/**
    Create and place `num` contiguous `T` objects given by `list` at `ptr`.
    @arg
    - `ptr`: the pointer to the first element;
    - `list`: the initializer list.
    @ret
    - `[null, err]` if `ptr` is `null` or `num` is `0`;
    - `[ptr, none]` otherwise.
    @pre
    - Sufficient, properly aligned storage at `ptr`.
    @nota
    - The elements are copied.
**/
template<typename T, typename U>
cons fn init_ls(mutaptr ptr, initls<U> lst) noexce -> ErrorCode
    where is_ctble<T, U const&>
{
    if (ptr == null) {
        return Invalid_Ptr;
    }

    if (lst.size() == 0) {
        return Invalid_Arg;
    }

    isize i = 0;
    for (auto const& elem : lst) {
        ::new (cast(T*, ptr) + i++) T(elem);
    }
    return null; 
}

/**
    Uninitializes `num` contiguous `T` objects at `ptr`.
    @arg
    - `T`: the type of the objects.
    @arg
    - `ptr`: the pointer to the objects.
    - `num`: the number of objects.
    @ret
    - `[null, err]` if `ptr == null` or `num == 0`,
    - `[empty, none]` otherwise.
    @pre
    - `ptr` was obtained from `alloc_aligned<T>`.
    - `ptr` has `num` elements.
**/
template<typename T>
nodisc fn deinit_ty(T* ptr, isize num) noexce -> ErrorCode
{
    if (ptr == null) {
        return Invalid_Ptr;
    }
    if (num == 0) {
        return Invalid_Arg;
    }
    if constexpr (is_zero_initble<T>) {
        return none;
    } else {
        for (isize i = 0; i < num; i++) {
            ptr[num].~T();
        }
        return none;
    }
}

}       // namespace mem 
}       // namespace cx
#endif  // LIBCX_MEM_COMMON_HH
