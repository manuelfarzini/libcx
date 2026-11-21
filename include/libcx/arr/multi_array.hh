#ifndef CX_ARR_MULTI_ARRAY_HH
#define CX_ARR_MULTI_ARRAY_HH

#include "libcx/concept/type.hh"
#include "libcx/conf/prelude.hh"
#include "libcx/arr/macro.hh"
#include "libcx/mem/allocator.hh"
#include "libcx/mem/multi.hh"
#include "libcx/uti/ownership.hh"
#include "libcx/uti/tuple.hh"
#include "libcx/uti/typeseq.hh"

namespace cx {
inline namespace arr {

template<PlainPassiveValue... Ts> with (sizeof...(Ts) > 1)
struct MultiArray
{
    using Types = TypeSeq<Ts...>;
    using Size = isize;
    onedef glob cons isize rows = sizeof...(Ts);

    Tuple<Ts*...> ptrs;
    isize      len;
    isize      cap;
    Allocator  alc;
};

template<typename T> proposition is_multi_array = false;
template<PlainPassiveValue... Ts> proposition is_multi_array<MultiArray<Ts...>> = true;
template<typename T> concept SomeMultiArray = is_multi_array<T>;

template<PlainPassiveValue... Ts>
inln priv fn rep_ok(MultiArray<Ts...> const& arr) noexce -> b32
{
    if (arr.len < 0 || arr.cap < 0 || arr.len > arr.cap) {
        return false;
    }
    if (arr.cap == 0) {
        return [&]<isize... I>(IndexSeq<I...>) -> b32 {
            return ((get<I>(arr.ptrs) == null) && ...);
        }(index_seq<sizeof...(Ts)>{});
    }
    if (get<0>(arr.ptrs) == null || arr.alc.call == null ||
        uptr(get<0>(arr.ptrs)) % multi_align_of<Ts...>() != 0)
    {
        return false;
    }
    auto expected = multi_bind<Ts...>(get<0>(arr.ptrs), arr.cap);
    return [&]<isize... I>(IndexSeq<I...>) -> b32 {
        return ((get<I>(arr.ptrs) == get<I>(expected)) && ...);
    }(index_seq<sizeof...(Ts)>{});
}

template<PlainPassiveValue... Ts>
inln fn make_multi_array(
    isize      cap,
    Allocator  alc    = heap_allocator(),
    AllocFlag  flags  = AllocFlag_Zero
) noexce -> Res<MultiArray<Ts...>, ErrorCode> with (sizeof...(Ts) > 1)
{
    if (cap < 0) {
        return {MultiArray<Ts...>{{}, 0, 0, alc}, Invalid_Arg};
    }
    if (cap == 0) {
        return {MultiArray<Ts...>{{}, 0, 0, alc}, none};
    }
    auto [ptr, err] =
        multi_mem_alloc<Ts...>(alc, cap, flags) or_return {MultiArray<Ts...>{{}, 0, 0, alc}, err};
    auto arr = MultiArray<Ts...>{multi_bind<Ts...>(ptr, cap), 0, cap, alc};
    cx_assert(rep_ok(arr));
    return {arr, none};
}

template<PlainPassiveValue... Ts>
fn reserve(MultiArray<Ts...>& arr, isize new_cap) noexce -> ErrorCode
{
    if (new_cap <= arr.cap) {
        return none;
    }
    auto [ptr, err] =
        multi_mem_resize<Ts...>(arr.alc, get<0>(arr.ptrs), arr.cap, new_cap, arr.len) or_return err;
    arr.ptrs = multi_bind<Ts...>(ptr, new_cap);
    arr.cap = new_cap;
    cx_assert(rep_ok(arr));
    return none;
}

template<PlainPassiveValue... Ts>
fn resize(MultiArray<Ts...>& arr, isize new_len) noexce -> ErrorCode
{
    if (new_len < 0) {
        return Invalid_Arg;
    }
    auto err = reserve(arr, new_len) or_return err;
    if (new_len > arr.len) {
        multi_init_value<Ts...>(get<0>(arr.ptrs), arr.cap, arr.len, new_len - arr.len);
    }
    arr.len = new_len;
    cx_assert(rep_ok(arr));
    return none;
}

template<PlainPassiveValue... Ts>
fn shrink(MultiArray<Ts...>& arr) noexce -> ErrorCode
{
    if (arr.len == arr.cap) {
        return none;
    }
    if (arr.len == 0) {
        auto [_, err] = mem_free(arr.alc, get<0>(arr.ptrs)) or_return err;
        arr.ptrs = {};
        arr.cap = 0;
        return none;
    }
    auto [ptr, err] =
        multi_mem_resize<Ts...>(arr.alc, get<0>(arr.ptrs), arr.cap, arr.len, arr.len) or_return err;
    arr.ptrs = multi_bind<Ts...>(ptr, arr.len);
    arr.cap = arr.len;
    cx_assert(rep_ok(arr));
    return none;
}

template<PlainPassiveValue... Ts>
fn free(MultiArray<Ts...>& arr) noexce -> ErrorCode
{
    if (get<0>(arr.ptrs) != null) {
        auto [_, err] = mem_free(arr.alc, get<0>(arr.ptrs)) or_return err;
    }
    arr.ptrs = {};
    arr.len = 0;
    arr.cap = 0;
    cx_assert(rep_ok(arr));
    return none;
}

template<isize Row, PlainPassiveValue... Ts>
inln fn get_row(MultiArray<Ts...>& arr) noexce -> auto* with (Row >= 0 && Row < sizeof...(Ts))
{
    return get<Row>(arr.ptrs);
}

template<isize Row, PlainPassiveValue... Ts>
inln fn get_row(MultiArray<Ts...> const& arr) noexce -> auto const*
    with (Row >= 0 && Row < sizeof...(Ts))
{
    return get<Row>(arr.ptrs);
}

template<isize Row, SomeMultiArray Arr>
inln fn get_elm(Arr& arr, isize col) noexce -> auto&
{
    cx_assert(col >= 0 && col < arr.len);
    return get_row<Row>(arr)[col];
}

template<isize Row, SomeMultiArray Arr>
inln fn get_elm(Arr const& arr, isize col) noexce -> auto const&
{
    cx_assert(col >= 0 && col < arr.len);
    return get_row<Row>(arr)[col];
}

template<SomeMultiArray Arr>
inln fn get_elm(Arr& arr, isize row, isize col) noexce -> mutaptr
{
    cx_assert(col >= 0 && col < arr.len);
    mutaptr ptr = null;
    [&]<isize... I>(IndexSeq<I...>) {
        ((row == I ? (ptr = get<I>(arr.ptrs) + col, true) : false) || ...);
    }(index_seq<Arr::rows>{});
    return ptr;
}

template<SomeMultiArray Arr>
inln fn get_elm(Arr const& arr, isize row, isize col) noexce -> readptr
{
    cx_assert(col >= 0 && col < arr.len);
    readptr ptr = null;
    [&]<isize... I>(IndexSeq<I...>) {
        ((row == I ? (ptr = get<I>(arr.ptrs) + col, true) : false) || ...);
    }(index_seq<Arr::rows>{});
    return ptr;
}

template<SomeMultiArray Arr, typename... Ts>
fn append(Arr& arr, Ts&&... values) noexce -> ErrorCode with (sizeof...(Ts) == Arr::rows)
{
    if (arr.len == arr.cap) {
        if (arr.cap > ISIZE_MAX / 2) {
            return Invalid_Arg;
        }
        auto err = reserve(arr, arr.cap > 0 ? arr.cap * 2 : 1) or_return err;
    }
    [&]<isize... I>(IndexSeq<I...>) {
        ((get<I>(arr.ptrs)[arr.len] = forward<Ts>(values)), ...);
    }(index_seq<Arr::rows>{});
    arr.len++;
    cx_assert(rep_ok(arr));
    return none;
}

#if defined(CX_TEST_MULTI_ARRAY)
CX_TEST_DEFINE(multi_array_basic)
{
    auto [arr, err] = make_multi_array<i32, f64>(2);
    cx_assert_hard(!err, "Validation failed");
    cx_assert_hard(!append(arr, i32(10), f64(1.5)), "Validation failed");
    cx_assert_hard(!reserve(arr, 8), "Validation failed");
    cx_assert_hard(get_elm<0>(arr, 0) == 10 && get_elm<1>(arr, 0) == 1.5, "Validation failed");
    cx_assert_hard(!resize(arr, 3), "Validation failed");
    cx_assert_hard(get_elm<0>(arr, 2) == 0, "Validation failed");
    cx_assert_hard(!shrink(arr), "Validation failed");
    cx_assert_hard(arr.len == 3 && arr.cap == 3, "Validation failed");
    cx_assert_hard(!free(arr), "Validation failed");
}
#endif

}       // namespace arr
}       // namespace cx
#endif  // CX_ARR_MULTI_ARRAY_HH
