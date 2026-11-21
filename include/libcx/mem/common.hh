/** @file libcx/mem/common.hh **/

#ifndef LIBCX_MEM_COMMON_HH
#define LIBCX_MEM_COMMON_HH

#include "libcx/conf/prelude.hh"
#include "libcx/conf/type.hh"
#include "libcx/trait.hh"
#include "libcx/concept.hh"
#include "libcx/uti/ownership.hh"
#include "libcx/uti/tuple.hh"

#ifndef CX_ENABLE_PLACEMENT
#    define CX_ENABLE_PLACEMENT 0
#endif

#if CX_ENABLE_PLACEMENT

// NOTE:(manu) Custom placement new/delete must not be used unless explicitly requested.

struct PlaceTag
{};

CX_HIDE nodisc cons fn operator new(usize, mutaptr __p, PlaceTag) noexce -> mutaptr
{
    return __p;
}
CX_HIDE cons fn operator delete(mutaptr, mutaptr, PlaceTag) noexce -> void
{
}

#    ifndef cx_new
#        define cx_new(__p) ::new (__p, PlaceTag{})
#    endif

#endif

namespace cx {
inline namespace mem {

onedef cons isize PTR_SIZE   = size_of(mutaptr);
onedef cons isize PTR_ALIGN  = align_of(mutaptr);
onedef cons isize MAX_SIZE   = size_of(max_align_t);
onedef cons isize MAX_ALIGN  = align_of(max_align_t);
onedef cons isize DEF_ALIGN  = 2 * MAX_ALIGN;

static_assert(MAX_ALIGN % PTR_ALIGN == 0, "`MAX_ALIGN` must be a multiple of `ptr_align`");
static_assert((MAX_ALIGN & (MAX_ALIGN - 1)) == 0, "`MAX_ALIGN` must be a power of 2");

/** Predicate that is `true` if `Tp` requires more alignment than `max_align_t`. **/
template<typename Tp> proposition is_over_aligned = align_of(Tp) > align_of(max_align_t);

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
inln cons fn mem_copy(mutaptr cx_restrict dst, readptr cx_restrict src, isize size) noexce -> void
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
    with (not is_void<T>)
{
    cx_assume(uptr(dst) + num * size_of(T) <= uptr(src) ||
              uptr(src) + num * size_of(T) <= uptr(dst));
    mem_copy(mutaptr(dst), mutaptr(src), num * size_of(T));
}

/**
    Copies `size` bytes from `src` to `dst`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    @req
    - `src` and `dst` have `size` bytes.
**/
inln cons fn mem_move(mutaptr dst, readptr src, isize size) -> void
{
    ::memmove(dst, src, size);
}

/**
    Copies `num` elements of type `T` from `src` to `dst`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    @req
    - `src` and `dst` have `num` elements.
**/
template<typename T>
inln cons fn mem_move(T* dst, T const* src, isize num) noexce -> void with (not is_void<T>)
{
    mem_move(mutaptr(dst), mutaptr(src), num * size_of(T));
}

/**
    Takes `num` elements of type `T` from `src` to `dst`.
    If `T` is not move assignable, then behaves like `memcopy`.
    @arg
    - `dst`: the destination pointer.
    - `src`: the source pointer.
    @req
    - `src` and `dst` have `num` elements.
**/
template<CpOrMvAsble T>
cons fn mem_take(T* dst, T const* src, isize num) noexce -> void with (not is_void<T>)
{
    for (isize i = 0; i < num; i++) {
        dst[i] = take(src[i]);
    }
}

/**
    Sets `size` bytes of memory at `data` to `val`.
    @arg
    - `data`: pointer to the memory to be set.
    - `val`: value to set the memory to.
    - `size`: size of the memory in bytes.
    @req
    - `data` has at least `size` bytes.
**/
cons fn mem_set(mutaptr data, u8 val, isize size) -> ErrorCode
{
    // NOTE:(manu)
    //  - libc wrapper, actually is the fastest
    //  - found that is the fastest on my m1 macbook pro
    //    since it is implemented in arm64 asm
    //  - maybe on other platoforms I should use another version
    //  XXX:(manu) should I wrap directly the os calls?

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
    - `data`: pointer to the memory to be zeroed.
    - `size`: size of the memory in bytes.
    @ret
    - `ErrorCode`.
    @req
    - `data` has at least `size` bytes.
    - `data` is not null.
**/
cons fn mem_zero(mutaptr data, isize size) -> ErrorCode
{
    return mem_set(data, 0, size);
}

/**
    Zeroes `size` bytes of memory at `data`.
    @arg
    - `data`: pointer to the memory to be zeroed.
    - `size`: size of the memory in bytes.
    @ret
    - The generated `ErrorCode` if any, `none` otherwise.
    @req
    - `data` has at least `size` bytes.
    @inf
    - Intended for virtual memory that may already be mapped to a zero page.
**/
cons fn mem_zero_condition(mutaptr data, isize size) -> ErrorCode
{
    // NOTE:(manu)
    // gb.h memset inspired version
    // it's faster compared to the odin like one and it's much faster than
    // libc ::memset with 0s if the memory is already mapped to a 0 page
    // probably I can improve this even more under certain assumptions,
    // like using a u64 splat directly
    // TODO:(manu) what about using ::memset call conditionally?
    if (data == null)
        unlike
        {
            return Invalid_Ptr;
        }
    if (size <= 0)
        unlike
        {
            return Invalid_Arg;
        }

#define zero_if_not(x)                                                                             \
    if (x != 0)                                                                                    \
        unlike                                                                                     \
        {                                                                                          \
            x = 0;                                                                                 \
        }
#define ret_if_less(a, b)                                                                          \
    if (a < b) {                                                                                   \
        return none;                                                                               \
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
    auto d64 = cast(u64*, d);
    for (; size >= 32;) {
        if ((d64[0] | d64[1] | d64[2] | d64[3]) != 0) {
            d64[0] = 0;
            d64[1] = 0;
            d64[2] = 0;
            d64[3] = 0;
        }

        d += 32;
        size -= 32;
    }

    for (; size >= 4;) {
        zero_if_not(*cast(u32*, d));
        d += 4;
        size -= 4;
    }

#undef zero_if_not
#undef ret_if_less
    return none;
}

/**
    Value-initializes `num` contiguous objects of type `T` using a single
    value-initialized instance as a template.
    @arg
    - `ptr`: destination storage.
    - `num`: number of elements to initialize.
    @req
    - `ptr` is not null.
    - `num` is positive.
    - `ptr` refers to storage sufficient for `num` elements of type `T`.
    - `ptr` is properly aligned for `T`.
    @inf
    - The template value is initialized with `T{}` and copied using `mem_copy`.
    - Default member initializers are respected.
**/
template<PassiveValue T>
inln cons fn init_value(mutaptr ptr, isize num) noexce -> void
{
    cx_assert(ptr != null && num > 0);

    T value{};
    T* buf = cast(T*, ptr);

    for (isize i = 0; i < num; i++) {
        mem_copy(buf + i, &value, 1);
    }
}

/**
    Initializes `num` contiguous objects of type `T` using a single instance
    initialized from `args`.
    @arg
    - `ptr`: destination storage.
    - `num`: number of elements to initialize.
    - `args`: arguments used to initialize the template value.
    @req
    - `ptr` is not null.
    - `num` is positive.
    - `ptr` refers to storage sufficient for `num` elements of type `T`.
    - `ptr` is properly aligned for `T`.
    @inf
    - The template value is created once and replicated using `mem_copy`.
    - Arguments are taken by const reference because they are not consumed.
**/
template<PassiveValue T, typename... Args>
inln cons fn init_args(mutaptr ptr, isize num, Args const&... args) noexce -> void
{
    cx_assert(ptr != null && num > 0);

    T value{args...};
    T* buf = cast(T*, ptr);

    for (isize i = 0; i < num; i++) {
        mem_copy(buf + i, &value, 1);
    }
}

/**
    Initializes contiguous objects of type `T` from an initializer list.
    @arg
    - `ptr`: destination storage.
    - `lst`: list of values to copy.
    @req
    - `ptr` is not null.
    - `lst` is not empty.
    - `ptr` refers to storage sufficient for `lst.size()` elements of type `T`.
    - `ptr` is properly aligned for `T`.
    - Source and destination storage do not overlap.
    @inf
    - Elements are copied directly when `T` and `U` match.
    - Otherwise, each element is converted to `T` before copying.
**/
template<PassiveValue T, typename U>
inln cons fn init_list(mutaptr ptr, initls<U> lst) noexce -> void
{
    cx_assert(ptr != null && lst.size() > 0);

    T* buf = cast(T*, ptr);
    isize i = 0;

    for (auto const& elem : lst) {
        if cons (same_as<T, U>) {
            mem_copy(buf + i, &elem, 1);
        } else {
            T value{elem};
            mem_copy(buf + i, &value, 1);
        }
        i++;
    }
}

}       // namespace mem
}       // namespace cx
#endif  // LIBCX_MEM_COMMON_HH
