#ifndef CX_MEM_MULTI_HH
#define CX_MEM_MULTI_HH

#include "libcx/concept/lifetime.hh"
#include "libcx/conf/prelude.hh"
#include "libcx/mem/allocator.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/pointer.hh"
#include "libcx/uti/tuple.hh"

namespace cx {
inline namespace mem {

template<typename... Ts>
inln cons fn multi_align_of() noexce -> isize
{
    isize align = DEF_ALIGN;
    ((align = align < align_of(Ts) ? align_of(Ts) : align), ...);
    return align;
}

template<typename... Ts>
inln cons fn multi_size_of(isize num) noexce -> isize
{
    cx_assert(num >= 0);
    isize off = 0;
    ((off = align_up(off, align_of(Ts)), off += num * size_of(Ts)), ...);
    return off;
}

template<typename T, typename U>
inln cons fn multi_align_up_next(isize off, isize num) noexce -> isize
{
    return align_up(off + num * size_of(T), align_of(U));
}

template<typename... Ts>
inln cons fn multi_bind(mutaptr ptr, isize cap) noexce -> Tuple<Ts*...>
{
    auto bind_one = [&]<typename T>() -> T* {
        ptr = align_up<T>(ptr);
        T* row = cast(T*, ptr);
        ptr = row + cap;
        return row;
    };
    return {bind_one.template operator()<Ts>()...};
}

template<PassiveValue... Ts>
inln fn multi_mem_copy(
    mutaptr  dst,
    isize    dst_cap,
    readptr  src,
    isize    src_cap,
    isize    num
) noexce -> void
{
    cx_assert(num >= 0 && num <= dst_cap && num <= src_cap);
    isize dst_off = 0;
    isize src_off = 0;
    auto copy_one = [&]<typename T>() {
        dst_off = align_up(dst_off, align_of(T));
        src_off = align_up(src_off, align_of(T));
        if (num > 0) {
            mem_copy(ptr_add(dst, dst_off), cast(u8 const*, src) + src_off, num * size_of(T));
        }
        dst_off += dst_cap * size_of(T);
        src_off += src_cap * size_of(T);
    };
    (copy_one.template operator()<Ts>(), ...);
}

template<PassiveValue... Ts>
inln fn multi_mem_copy(mutaptr dst, readptr src, isize num) noexce -> void
{
    multi_mem_copy<Ts...>(dst, num, src, num, num);
}

template<PassiveValue... Ts>
inln priv fn multi_relayout(mutaptr ptr, isize old_cap, isize new_cap, isize num) noexce -> void
{
    isize sizes[] = {size_of(Ts)...};
    isize aligns[] = {align_of(Ts)...};
    isize old_offsets[sizeof...(Ts)]{};
    isize new_offsets[sizeof...(Ts)]{};
    isize old_off = 0;
    isize new_off = 0;
    for (isize i = 0; i < isize(sizeof...(Ts)); i++) {
        old_offsets[i] = align_up(old_off, aligns[i]);
        new_offsets[i] = align_up(new_off, aligns[i]);
        old_off = old_offsets[i] + old_cap * sizes[i];
        new_off = new_offsets[i] + new_cap * sizes[i];
    }
    for (isize step = 0; step < isize(sizeof...(Ts)); step++) {
        isize i = new_cap > old_cap ? isize(sizeof...(Ts)) - step - 1 : step;
        if (num > 0) {
            mem_move(ptr_add(ptr, new_offsets[i]), ptr_add(ptr, old_offsets[i]), num * sizes[i]);
        }
    }
}

template<PassiveValue... Ts>
inln fn multi_init_value(mutaptr ptr, isize cap, isize beg, isize num) noexce -> void
{
    cx_assert(ptr != null && beg >= 0 && num > 0 && beg <= cap && num <= cap - beg);
    isize off = 0;
    auto init_one = [&]<typename T>() {
        off = align_up(off, align_of(T));
        init_value<T>(byteptr(ptr) + off + beg * size_of(T), num);
        off += cap * size_of(T);
    };
    (init_one.template operator()<Ts>(), ...);
}

template<PassiveValue... Ts>
inln fn multi_mem_zero(mutaptr ptr, isize cap, isize beg, isize num) noexce -> void
{
    cx_assert(ptr != null && beg >= 0 && num > 0 && beg <= cap && num <= cap - beg);
    isize off = 0;
    auto zero_one = [&]<typename T>() {
        off = align_up(off, align_of(T));
        auto err = mem_zero(byteptr(ptr) + off + beg * size_of(T), num * size_of(T));
        cx_assert_hard(!err, "Validation failed");
        off += cap * size_of(T);
    };
    (zero_one.template operator()<Ts>(), ...);
}

template<typename... Ts>
inln priv fn multi_valid_capacity(isize cap) noexce -> b32
{
    if (cap <= 0) {
        return false;
    }
    isize off = 0;
    b32 valid = true;
    auto check_one = [&]<typename T>() {
        isize padding = align_of(T) - 1;
        if (!valid || off > ISIZE_MAX - padding) {
            valid = false;
            return;
        }
        off = align_up(off, align_of(T));
        if (cap > (ISIZE_MAX - off) / size_of(T)) {
            valid = false;
            return;
        }
        off += cap * size_of(T);
    };
    (check_one.template operator()<Ts>(), ...);
    return valid;
}

template<PassiveValue... Ts>
inln fn multi_mem_alloc(
    Allocator  a,
    isize      cap,
    AllocFlag  flags  = AllocFlag_Default
) noexce -> Res<mutaptr, ErrorCode>
{
    if (!multi_valid_capacity<Ts...>(cap)) {
        return {null, Invalid_Arg};
    }
    return mem_alloc(a, multi_size_of<Ts...>(cap), multi_align_of<Ts...>(), flags);
}

template<PassiveValue... Ts>
inln fn multi_mem_resize(
    Allocator  a,
    mutaptr    ptr,
    isize      old_cap,
    isize      new_cap,
    isize      len,
    AllocFlag  flags  = AllocFlag_None
) noexce -> Res<mutaptr, ErrorCode>
{
    if (!multi_valid_capacity<Ts...>(new_cap) || len < 0 || len > old_cap || len > new_cap) {
        return {ptr, Invalid_Arg};
    }
    if (ptr == null) {
        return multi_mem_alloc<Ts...>(a, new_cap, flags);
    }
    b32 compact = new_cap < old_cap;
    if (compact) {
        multi_relayout<Ts...>(ptr, old_cap, new_cap, len);
    }
    auto [buf, err] =
        mem_resize(a, ptr, multi_size_of<Ts...>(old_cap), multi_size_of<Ts...>(new_cap),
                   multi_align_of<Ts...>(), AllocFlag_None);
    if (err) {
        if (compact) {
            multi_relayout<Ts...>(ptr, new_cap, old_cap, len);
        }
        return {ptr, err};
    }
    if (!compact && new_cap != old_cap) {
        multi_relayout<Ts...>(buf, old_cap, new_cap, len);
    }
    if ((flags & AllocFlag_Zero) && new_cap > len) {
        multi_mem_zero<Ts...>(buf, new_cap, len, new_cap - len);
    }
    return {buf, none};
}

#if defined(CX_TEST_MEM_MULTI)
CX_TEST_DEFINE(mem_multi_basic)
{
    auto a = heap_allocator();
    auto [ptr, err] = multi_mem_alloc<i32, f64>(a, 2);
    cx_assert_hard(!err, "Allocation failed");
    multi_init_value<i32, f64>(ptr, 2, 0, 2);
    auto rows = multi_bind<i32, f64>(ptr, 2);
    get<0>(rows)[0] = 42;
    get<1>(rows)[0] = 3.5;
    auto [buf, resize_err] = multi_mem_resize<i32, f64>(a, ptr, 2, 8, 2);
    cx_assert_hard(!resize_err, "Resize failed");
    rows = multi_bind<i32, f64>(buf, 8);
    cx_assert_hard(get<0>(rows)[0] == 42 && get<1>(rows)[0] == 3.5, "Live data changed");
    auto [_, free_err] = mem_free(a, buf);
    cx_assert_hard(!free_err, "Free failed");
}
#endif

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_MULTI_HH
