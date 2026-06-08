/** @file libcx/arr/multi_array.hh **/

#ifndef CX_ARR_MULTI_ARRAY_HH
#define CX_ARR_MULTI_ARRAY_HH

#include "libcx/conf/macro.hh"
#include "libcx/arr/static_array.hh"
#include "libcx/mem/multi.hh"
#include "libcx/concept/multi.hh"
#include "libcx/traits/multi.hh"
#include "libcx/uti/typeseq.hh"
#include "libcx/uti/utilities.hh"

namespace cx {
inline namespace arr {

#ifndef TypesIn
    #define TypesIn(arr) cx::rm_cvref<declt(arr)>::Types
#endif
#ifndef base_ptr
    #define base_ptr(arr) arr.ptrs[0]
#endif
///////////////////////////////////////////
// Multi Array

/**
    TODO
**/
template<SomeAllocator A, PlainZeroInitble... Ts> requires (va_size_of<Ts...> > 1)
struct MultiArray {
    using Types = TypeSeq<Ts...>;
    using Self = MultiArray<A, Ts...>;
    using Alc = A;
    onedef glob cons isize rows = sizeof...(Ts);
    onedef glob cons StaticArray<isize, rows> sizes{size_of(Ts)...};
    onedef glob cons StaticArray<isize, rows> aligns{align_of(Ts)...};

    StaticArray<mutaptr, rows> ptrs;
    isize len{};
    isize cap{};
    Alc alc{};  // XXX: handle initialization
};

CX_CONCEPT_GEN_TEMPL(
    MultiArray, is_multi_array, SomeMultiArray, VA_(typename... Ts), VA_(Ts...)
);
#define Multi_Array cx::arr::SomeMultiArray auto

///////////////////////////////////////////
// Base operations

template<SomeMultiArray Arr>
fn get_ptr(Arr& arr, isize row, isize col) -> mutaptr
{
    return ptr_add(arr.ptrs[row], col * Arr::sizes[row]);
}

template<isize Row, SomeMultiArray Arr>
fn get(Arr& arr, isize col) -> TypeAt<Row, typename Arr::Types>&
{
    using T = TypeAt<Row, typename Arr::Types>;
    T* row = cast(T*, arr.ptrs[Row]);
    return row[col];
}

///////////////////////////////////////////
// Allocator

template<PlainZeroInitble... Ts>
fn reserve(MultiArray<HeapAllocator, Ts...>& arr, isize wanted_cap) -> ErrorCode
{
    if (wanted_cap <= arr.cap) {
        return null;
    }
    return resize(arr, wanted_cap);
}

template<PlainZeroInitble... Ts>
fn ensure_capacity(MultiArray<HeapAllocator, Ts...>& arr, isize req_len) -> ErrorCode
{
    if (req_len <= arr.cap) {
        return null;
    }
    isize new_cap = arr.cap == 0 ? 8 : arr.cap;
    while (new_cap < req_len) {
        new_cap *= 2;
    }
    return reserve(arr, new_cap);
}

template<PlainZeroInitble... Ts>
fn alloc(
    MultiArray<HeapAllocator, Ts...>&    arr,
    isize                                new_cap,
    isize                                align     =  DEF_ALIGN,
    u32                                  flags     =  AllocFlags_Default
) -> ErrorCode {
    isize ALIGN = max(align, multi_align_of<Ts...>());
    isize SIZE  = multi_size_of<Ts...>(new_cap);

    auto [ptr, err] = aligned_alloc(arr.alc, SIZE, ALIGN, flags) or_return err;

    mutaptr p = ptr;
    clos bind_one = [&]<isize I, typename T>() inln_clos -> void {
        p = align_up<T>(p);
        T* beg = cast(T*, p);
        arr.ptrs[I] = beg;
        p = beg + new_cap;
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (bind_one.template operator()<I, Ts>(), ...);
    }(index_seq_va<Ts...>{});

    arr.len = 0;
    arr.cap = new_cap;
    return null;
}

template<PlainZeroInitble... Ts>
fn resize(
    MultiArray<HeapAllocator, Ts...>&    arr,
    isize                                new_cap,
    isize                                align     =  DEF_ALIGN,
    u32                                  flags     =  AllocFlags_Default
) -> ErrorCode {
    cx_assume(arr.cap <= new_cap);
    cx_assume(arr.len <= new_cap);

    mutaptr old_base = base_ptr(arr);
    auto old_ptrs = arr.ptrs;
    isize old_len = arr.len;

    isize ALIGN = max(align, multi_align_of<Ts...>());
    isize SIZE  = multi_size_of<Ts...>(new_cap);

    auto [ptr, err] = aligned_alloc(arr.alc, SIZE, ALIGN, flags & ~AllocFlags_Zero)
        or_return err;

    mutaptr p = ptr;
    clos resize_one = [&]<isize I, typename T>() inln_clos -> void {
        p = align_up<T>(p);
        T* dst = cast(T*, p);
        T* src = cast(T*, old_ptrs[I]);
        arr.ptrs[I] = dst;
        if (old_len > 0) {
            mem_copy(dst, src, old_len);
        }
        if (flags & AllocFlags_Zero) {
            mem_zero(dst + old_len, (new_cap - old_len) * size_of(T));
        }
        p = dst + new_cap;
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (resize_one.template operator()<I, Ts>(), ...);
    }(index_seq_va<Ts...>{});

    arr.cap = new_cap;
    if (old_base != null) {
        return aligned_free(arr.alc, old_base);
    }

    return null;
}

template<PlainZeroInitble... Ts>
fn free(MultiArray<HeapAllocator, Ts...>& arr) -> ErrorCode
{
    mutaptr ptr = base_ptr(arr);
    if (ptr != null) {
        aligned_free(arr.alc, ptr);
    }
    for (isize i = 0; i < arr.rows; i++) {
        arr.ptrs[i] = null;
    }
    arr.len = 0;
    arr.cap = 0;
    return null;
}

template<SomeMultiArray Arr, typename... Ts>
fn push_back(Arr& arr, Ts&&... els) -> ErrorCode
    where (multi_same_or_ref<typename Arr::Types, TypeSeq<Ts...>>)
{
    ErrorCode err = ensure_capacity(arr, arr.len + 1) or_return err;

    clos push_one = [&]<isize I>(auto&& elm) inln_clos -> void {
        using T = TypeAt<I, typename Arr::Types>;
        T* dst = cast(T*, ptr_add(arr.ptrs[I], arr.len * Arr::sizes[I]));
        *dst = forward<decltype(elm)>(elm);
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (push_one.template operator()<I>(forward<Ts>(els)), ...);
    }(index_seq_va<Ts...>{});

    arr.len += 1;
    return null;
}

////////////////////////////////////////////
// Testing

// #define CX_TEST_MULTI_ARRAY 1
#if CX_TEST_MULTI_ARRAY

CX_TEST_DEFINE(multi_array_base)
{
    using namespace cx;

    using Arr = MultiArray<HeapAllocator, i32, f64>;

    Arr arr{};

    ErrorCode err = alloc(arr, 2);
    assert(err == null);
    assert(arr.len == 0);
    assert(arr.cap == 2);
    assert(arr.ptrs[0] != null);
    assert(arr.ptrs[1] != null);

    err = push_back(arr, i32(10), f64(1.5));
    assert(err == null);
    assert(arr.len == 1);
    assert(arr.cap == 2);

    err = push_back(arr, i32(20), f64(2.5));
    assert(err == null);
    assert(arr.len == 2);
    assert(arr.cap == 2);

    err = push_back(arr, i32(30), f64(3.5));
    assert(err == null);
    assert(arr.len == 3);
    assert(arr.cap >= 3);

    assert(get<0>(arr, 0) == i32(10));
    assert(get<0>(arr, 1) == i32(20));
    assert(get<0>(arr, 2) == i32(30));

    assert(get<1>(arr, 0) == f64(1.5));
    assert(get<1>(arr, 1) == f64(2.5));
    assert(get<1>(arr, 2) == f64(3.5));

    get<0>(arr, 1) = i32(200);
    get<1>(arr, 1) = f64(22.5);

    assert(get<0>(arr, 1) == i32(200));
    assert(get<1>(arr, 1) == f64(22.5));

    i32* p0 = cast(i32*, get_ptr(arr, 0, 1));
    f64* p1 = cast(f64*, get_ptr(arr, 1, 1));

    assert(*p0 == i32(200));
    assert(*p1 == f64(22.5));

    *p0 = i32(300);
    *p1 = f64(33.5);

    assert(get<0>(arr, 1) == i32(300));
    assert(get<1>(arr, 1) == f64(33.5));

    err = free(arr);
    assert(err == null);
    assert(arr.len == 0);
    assert(arr.cap == 0);
    assert(arr.ptrs[0] == null);
    assert(arr.ptrs[1] == null);

    puts("test_multi_array_base: ok");
}

#endif  // CX_TEST_MULTI_ARRAY



}       // namespace arr
}       // namespace cx
#endif  // CX_ARR_MULTI_ARRAY_HH
