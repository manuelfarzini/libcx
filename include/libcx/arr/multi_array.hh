/** @file libcx/arr/multi_array.hh **/

#ifndef CX_ARR_MULTI_ARRAY_HH
#define CX_ARR_MULTI_ARRAY_HH

#include "libcx/conf/macro.hh"
#include "libcx/arr/static_array.hh"
#include "libcx/mem/multi.hh"
#include "libcx/concept/multi.hh"
#include "libcx/traits/multi.hh"
#include "libcx/uti/typeseq.hh"
#include "libcx/uti/tuple.hh"
#include "libcx/uti/utilities.hh"

namespace cx {
inline namespace arr {

#ifndef types_in
    #define types_in(arr) cx::rm_cvref<declt(arr)>::Types
#endif
#ifndef base_ptr
    #define base_ptr(arr) cast(mutaptr, get<0>((arr).ptrs))
#endif

///////////////////////////////////////////
// Multi Array

/** XXX:
    A dynamic Struct-Of-Arrays container.
    @desc
    - Stores one typed row pointer for each `Ts` in a tuple.
    - Uses compile-time row access as the preferred access path.
    - Layout: `[T0[cap]][pad][T1[cap]][pad]...[Tn[cap]]`.
    @rep
    - `ptrs` stores the typed begin pointer of each row.
    - `len` is the number of initialized columns.
    - `cap` is the number of allocated columns per row.
    - `alc` is the allocator instance.
    @nota
    - Runtime row access is supported, but it is intended as a fallback path.
**/
template<SomeAllocator A, PlainZeroInitble... Ts> requires (va_size_of<Ts...> > 1)
struct MultiArray {
    using Types = TypeSeq<Ts...>;
    using Self = MultiArray<A, Ts...>;
    using Alc = A;
    onedef glob cons isize rows = sizeof...(Ts);
    onedef glob cons StaticArray<isize, rows> sizes{size_of(Ts)...};
    onedef glob cons StaticArray<isize, rows> aligns{align_of(Ts)...};

    Tuple<Ts*...> ptrs;
    isize len{};
    isize cap{};
    Alc alc{};  // XXX: handle initialization
};

CX_CONCEPT_GEN_TEMPL(MultiArray, is_multi_array, SomeMultiArray, 
                     VA_(SomeAllocator A, typename... Ts), VA_(A, Ts...));
#define Multi_Array cx::arr::SomeMultiArray auto

////////////////////////////////////////////
// Macro

#define CX__MULTI_ROW_TYPE(pair) CX__MULTI_ROW_TYPE_ pair
#define CX__MULTI_ROW_TYPE_(T, name) T

#define CX__MULTI_ROW_NAME(Name, pair) CX__MULTI_ROW_NAME_(Name, pair)
#define CX__MULTI_ROW_NAME_(Name, pair) CX__MULTI_ROW_NAME__(Name, VA_ pair)
#define CX__MULTI_ROW_NAME__(Name, T, name) CX_JOIN3(Name, _, name),

#define CX_DEFINE_MULTI_ARRAY(Name, ...)                            \
    enum {                                                          \
        CX_FOR_EACH_WITH_ARG(CX__MULTI_ROW_NAME, Name, __VA_ARGS__) \
    };                                                    \
    using Name = MultiArray<                              \
        CX_FOR_EACH_COMMA(CX_MULTI_ROW_TYPE, __VA_ARGS__) \
    >;

///////////////////////////////////////////
// Base operations

/**
    Returns the typed pointer to row `Row`.
    @arg
    - `arr`: the multi-array.
    @ret
    - The typed pointer to row `Row`.
**/
template<isize Row>
fn get_row_ptr(SomeMultiArray auto& arr) -> TypeAt<Row, typename types_in(arr)>*
{
    return get<Row>(arr.ptrs);
}

/**
    Returns an erased pointer to the element at runtime row `row` and column `col`.
    @arg
    - `arr`: the multi-array.
    - `row`: the runtime row index.
    - `col`: the runtime column index.
    @ret
    - The erased pointer to the selected element.
    - `null` if `row` is outside the valid range.
    @pre
    - `col` is a valid column index.
    @nota
    - Runtime row access is dispatched over the typed tuple rows.
    - Intended for rare dynamic access, not for hot per-element loops.
**/
fn get_elm_ptr(SomeMultiArray auto& arr, isize row, isize col) -> mutaptr
{
    mutaptr ptr = null;
    clos select_one = [&]<isize I>() inln_clos -> void {  // scan the rows
        if (row != I) {
            return;
        }
        ptr = mutaptr(get_row_ptr<I>(arr) + col);
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (select_one.template operator()<I>() || ...);
    }(index_seq<arr.rows>{});

    return ptr;
}

/**
    Returns a typed reference to the element at compile-time row `Row` and column `col`.
    @arg
    - `arr`: the multi-array.
    - `col`: the column index.
    @ret
    - The selected typed element.
    @pre
    - `col` is a valid column index.
**/
template<isize Row>
fn get_elm(SomeMultiArray auto& arr, isize col) -> TypeAt<Row, typename types_in(arr)>&
{
    return get_row_ptr<Row>(arr)[col];
}

///////////////////////////////////////////
// Allocator

/**
    Ensures that the capacity is at least `wanted_cap`.
    @arg
    - `arr`: the multi-array.
    - `wanted_cap`: the requested capacity.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
**/
template<PlainZeroInitble... Ts>
fn reserve(MultiArray<HeapAllocator, Ts...>& arr, isize wanted_cap) -> ErrorCode
{
    if (wanted_cap <= arr.cap) {
        return null;
    }
    return resize(arr, wanted_cap);
}

/**
    Ensures that the capacity is enough for `req_len` elements.
    @arg
    - `arr`: the multi-array.
    - `req_len`: the requested logical length.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
    @nota
    - The capacity grows by repeated doubling.
**/
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

/**
    Allocates backing storage for all rows.
    @arg
    - `arr`: the multi-array to initialize.
    - `new_cap`: the allocated capacity per row.
    - `align`: the minimum requested allocation alignment.
    - `flags`: the allocation flags.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
    @nota
    - Binds each typed row pointer inside `arr.ptrs`.
    - Resets `len` to `0` and sets `cap` to `new_cap`.
**/
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
        get_elm<I>(arr.ptrs) = beg;
        p = beg + new_cap;
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (bind_one.template operator()<I, Ts>(), ...);
    }(index_seq_va<Ts...>{});

    arr.len = 0;
    arr.cap = new_cap;
    return null;
}

/**
    Reallocates backing storage for all rows.
    @arg
    - `arr`: the multi-array to resize.
    - `new_cap`: the new allocated capacity per row.
    - `align`: the minimum requested allocation alignment.
    - `flags`: the allocation flags.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
    @pre
    - `new_cap` is greater than or equal to `arr.cap`.
    - `new_cap` is greater than or equal to `arr.len`.
    @nota
    - Copies the previous `len` elements for each row.
    - Rebinds each typed row pointer inside `arr.ptrs`.
    - Zeroes the newly allocated tail when `AllocFlags_Zero` is set.
**/
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
        T* src = get_elm<I>(old_ptrs);
        get_elm<I>(arr.ptrs) = dst;
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
    return aligned_free(arr.alc, old_base);
}

/**
    Frees the backing allocation.
    @arg
    - `arr`: the multi-array to release.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
    @nota
    - Clears all typed row pointers and resets length/capacity.
**/
template<PlainZeroInitble... Ts>
fn free(MultiArray<HeapAllocator, Ts...>& arr) -> ErrorCode
{
    mutaptr ptr = base_ptr(arr);
    ErrorCode err = aligned_free(arr.alc, ptr) or_return err;

    clos clear_one = [&]<isize I>() inln_clos -> void {
        get_elm<I>(arr.ptrs) = null;
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (clear_one.template operator()<I>(), ...);
    }(index_seq<MultiArray<HeapAllocator, Ts...>::rows>{});

    arr.len = 0;
    arr.cap = 0;
    return null;
}

/**
    Appends one logical column.
    @arg
    - `arr`: the multi-array.
    - `els`: one element for each row.
    @ret
    - The generated `ErrorCode` if any, `null` otherwise.
**/
template<SomeMultiArray Arr, typename... Ts>
fn append(Arr& arr, Ts&&... els) -> ErrorCode
    where (multi_same_or_ref<typename Arr::Types, TypeSeq<Ts...>>)
{
    ErrorCode err = ensure_capacity(arr, arr.len + 1) or_return err;

    clos append_one = [&]<isize I>(auto&& elm) inln_clos -> void {
        get_row_ptr<I>(arr)[arr.len] = forward<decltype(elm)>(elm);
    };

    [&]<isize... I>(IndexSeq<I...>) inln_clos -> void {
        (append_one.template operator()<I>(forward<Ts>(els)), ...);
    }(index_seq_va<Ts...>{});

    arr.len += 1;
    return null;
}

CX_TEST_DEFINE(multi_array_define_macro)
{
    //  CX_DECLARE_MULTI_ARRAY(position, A, f64, x, f64, y, f64, z);
    //  =>
    //      using position_x_t = distinct<f64>; (lambda trick)
    //      using position_y_t = distinct<f64>;
    //      using position_z_t = distinct<f64>;
    //      using multi_position = MultiArray<A, position_x_t, position_y_t, position_z_t>;
    //      enum {position_x, position_y, position_z}
    //      multi_position positions{};
    //      append(positions, 1.0, 2.0, 3.0);
    //      assert(get<position_x>(positions) == 1.0);
    //      assert(get<position_y>(positions) == 2.0);
    //      assert(get<position_z>(positions) == 3.0);
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
    assert(get<0>(arr.ptrs) != null);
    assert(get<1>(arr.ptrs) != null);

    err = append(arr, i32(10), f64(1.5));
    assert(err == null);
    assert(arr.len == 1);
    assert(arr.cap == 2);

    err = append(arr, i32(20), f64(2.5));
    assert(err == null);
    assert(arr.len == 2);
    assert(arr.cap == 2);

    err = append(arr, i32(30), f64(3.5));
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
    assert(get<0>(arr.ptrs) == null);
    assert(get<1>(arr.ptrs) == null);
}

CX_TEST_DEFINE(multi_array_runtime_ptr)
{
    using namespace cx;
    using Arr = MultiArray<HeapAllocator, i32, f64, u8>;

    Arr arr{};

    ErrorCode err = alloc(arr, 1);
    assert(err == null);
    assert(arr.len == 0);
    assert(arr.cap == 1);
    assert(get<0>(arr.ptrs) != null);
    assert(get<1>(arr.ptrs) != null);
    assert(get<2>(arr.ptrs) != null);

    err = append(arr, i32(10), f64(1.5), u8(1));
    assert(err == null);
    assert(arr.len == 1);
    assert(arr.cap == 1);

    err = append(arr, i32(20), f64(2.5), u8(2));
    assert(err == null);
    assert(arr.len == 2);
    assert(arr.cap >= 2);

    assert(get<0>(arr, 0) == i32(10));
    assert(get<0>(arr, 1) == i32(20));
    assert(get<1>(arr, 0) == f64(1.5));
    assert(get<1>(arr, 1) == f64(2.5));
    assert(get<2>(arr, 0) == u8(1));
    assert(get<2>(arr, 1) == u8(2));

    mutaptr raw0 = get_ptr(arr, 0, 1);
    mutaptr raw1 = get_ptr(arr, 1, 1);
    mutaptr raw2 = get_ptr(arr, 2, 1);

    assert(raw0 != null);
    assert(raw1 != null);
    assert(raw2 != null);

    i32* p0 = cast(i32*, raw0);
    f64* p1 = cast(f64*, raw1);
    u8*  p2 = cast(u8*,  raw2);

    assert(*p0 == i32(20));
    assert(*p1 == f64(2.5));
    assert(*p2 == u8(2));

    *p0 = i32(200);
    *p1 = f64(22.5);
    *p2 = u8(22);

    assert(get<0>(arr, 1) == i32(200));
    assert(get<1>(arr, 1) == f64(22.5));
    assert(get<2>(arr, 1) == u8(22));

    err = reserve(arr, 16);
    assert(err == null);
    assert(arr.len == 2);
    assert(arr.cap >= 16);

    assert(get<0>(arr, 0) == i32(10));
    assert(get<0>(arr, 1) == i32(200));
    assert(get<1>(arr, 0) == f64(1.5));
    assert(get<1>(arr, 1) == f64(22.5));
    assert(get<2>(arr, 0) == u8(1));
    assert(get<2>(arr, 1) == u8(22));

    get<0>(arr, 0) = i32(1000);
    get<1>(arr, 0) = f64(1000.5);
    get<2>(arr, 0) = u8(100);

    assert(*cast(i32*, get_ptr(arr, 0, 0)) == i32(1000));
    assert(*cast(f64*, get_ptr(arr, 1, 0)) == f64(1000.5));
    assert(*cast(u8*,  get_ptr(arr, 2, 0)) == u8(100));

    assert(get_ptr(arr, -1, 0) == null);
    assert(get_ptr(arr, 3, 0) == null);

    err = free(arr);
    assert(err == null);
    assert(arr.len == 0);
    assert(arr.cap == 0);
    assert(get<0>(arr.ptrs) == null);
    assert(get<1>(arr.ptrs) == null);
    assert(get<2>(arr.ptrs) == null);
}

CX_TEST_DEFINE(multi_array) {
    CX_TEST_CASE(multi_array_base);
    CX_TEST_CASE(multi_array_runtime_ptr);
}

#endif  // CX_TEST_MULTI_ARRAY

}       // namespace arr
}       // namespace cx
#endif  // CX_ARR_MULTI_ARRAY_HH
