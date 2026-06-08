/** @file libcx/arr/inline_array.hh **/

#ifndef CX_ARR_INLINE_ARRAY_HH
#define CX_ARR_INLINE_ARRAY_HH

#include "libcx/uti/utilities.hh"
#include "libcx/mem/common.hh"

namespace cx {
inline namespace arr {

/// Contiguous `buf`fer of `T` objects with a fixed `cap`acity `C`
/// and a `len`gth indicating current active elements.
template<typename Tp, usize Cap, typename Sz = usize>
struct InlineArray
{
    static_assert(Cap > 0, "Array capacity must be greater than zero");
    static_assert(is_zero_initble<Tp>, "Element type `T` must be zero initializable");
    CX_DEFINE_MEMBER_TYPES(Tp, Sz);
    using Self = InlineArray<Elem, Cap, Size>;

    inline static cons Size cap{Cap};
    Elem buf[Cap]{};
    Size len{};

    inln cons fn operator[](usize const i) noexce -> Elem& { return buf[i]; }
    inln cons fn operator[](usize const i) const noexce -> Elem const& { return buf[i]; }

    inln cons fn begin() noexce -> Iter { return buf; }
    inln cons fn end() noexce -> Iter { return buf + len; }
    inln cons fn begin() const noexce -> Kter { return buf; }
    inln cons fn end() const noexce -> Kter { return buf + len; }
};

CX_CONCEPT_GEN_TEMPL(InlineArray, is_inline_array, SomeInlineArray,
                     VA_(typename Tp, usize Cap, typename Sz), VA_(Tp, Cap, Sz));
#define Inline_Array cx::arr::SomeInlineArray auto

/// Equality operator.
cons fn operator==(Inline_Array const& a, Inline_Array const& b) noexce -> bool
{
    if (&a == &b) {
        return true;
    }
    if (a.len != b.len) {
        return false;
    }

    for (usize i = 0; i < a.len; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

/// Comparison operator.
cons fn operator<=>(Inline_Array const& a, Inline_Array const& b) noexce -> i8
{
    if (a.len < b.len) {
        return -1;
    }
    if (a.len > b.len) {
        return 1;
    }
    for (usize i = 0; i < a.len; i++) {
        if (a[i] < b[i]) {
            return -1;
        }
        if (a[i] > b[i]) {
            return 1;
        }
    }
    return 0;
}

// =========================================
// Inline Array public API
// =========================================

/// Returns `true` if the array is empty.
cons fn is_empty(Inline_Array arr) noexce -> bool
{
    return arr.len == 0;
}

/// Returns `true` if the array is full.
cons fn is_full(Inline_Array arr) noexce -> bool
{
    return arr.len == arr.cap;
}

/// Shifts the `arr`'s elements to the right from `i` to `j` by `off` positions.
/// Returns `true` if the shift was successful.
template<SomeInlineArray Arr, typename Size = SizeIn<Arr>>
inln cons fn shift_right(Arr& arr, Size i, Size j, Size off) noexce -> bool
{
    if (off == 0 || j >= arr.cap || i >= j || i + off > j) {
        return false;
    }

    for (; j >= i; j--) {
        arr[j] = arr[j - off];
    }

    return true;
}

/// Shifts the `arr`'s elements to the left from `i` to `j` by `off` positions.
/// Returns `true` if the shift was successful.
template<SomeInlineArray Arr, typename Size = SizeIn<Arr>>
inln cons fn shift_left(Arr& arr, Size i, Size j, Size off) noexce -> bool
{
    if (off == 0 || j >= arr.cap || i >= j || i + off > j) {
        return false;
    }
    for (; i < j - off; i++) {
        arr[i] = arr[i + off];
    }
    return true;
}

///
template<
    SomeInlineArray Arr,
    typename Size = SizeIn<Arr>,
    typename... Args>
cons fn push_back(Arr& arr, Size num, Args&&... args) noexce -> ErrorCode
    where (same_or_cvref<ElemIn<Arr>, Args>&&...)
{
    if (num == 0) {
        return false;
    }
    auto [_, err] = init_varg<ElemIn<Arr>>(&arr.buf[arr.len], num, args...) or_return Operation_Fail;
    arr.len += num;
    return none;
}

///
template<SomeInlineArray Arr, typename Elm = ElemIn<Arr>>
fn push_back(Arr& arr, initls<Elm> lst) noexce -> bool
{
    if (lst.size() == 0 && lst.size() + arr.len > arr.cap) {
        return false;
    }
    init_list<ElemIn<Arr>>(&arr.buf[arr.len], lst);
    arr.len += lst.size();
    return true;
}

///
template<SomeInlineArray Arr, typename Elm = ElemIn<Arr>>
fn pop_back(Arr& arr, Elm& res) noexce -> bool
{
    if (arr.len == 0) {
        return false;
    }
    res = arr[arr.len - 1];
    deinit_ty<ElemIn<Arr>>(&arr.buf[arr.len - 1]);  // trival dtble → nop
    arr.len--;
    return true;
}

template<SomeInlineArray Arr, typename Elm, OrderType Cmp = Lne>
cons fn find(Arr& arr, Elm&& elm, Cmp cmp = lne) noexce -> isize
    where is_total_ordered<ElemIn<Arr>> && same_or_cvref<ElemIn<Arr>, Elm>
{
    isize i;
    if constexpr (same_as<Cmp, Lne>) {
        i = 0;
        while (i < arr.len && cmp(arr[i], elm)) {
            i++;
        }
        if (i < arr.len && eq(arr[i], elm)) {
            return i;
        }

    } else if constexpr (same_as<Cmp, Gne>) {
        i = arr.len;
        while (i > 0 && cmp(arr[i - 1], elm)) {
            i--;
        }
        if (i > 0 && eq(arr[i - 1], elm)) {
            return i - 1;
        }
    }

    return -i - 1;
}

/// Find operation with `key` whose type may differ from the `ElemType`
/// in `arr` and using a custom comparator not provided by `cx::uti::`.
/// Assumes the array is sorted in ascending order.
/// Returns the insert position `-i - 1` if not found, otherwise `i`,
/// where `i` is the index.
// FIX:(manu) custom is_callable with parameters to add in the requires
// is_callable<Cmp, ElemIn<Arr>, PlainT<Key>>
template<
    SomeInlineArray Arr, typename Key, typename Cmp
> 
cons fn find(Arr& arr, Key&& key, Cmp cmp) noexce -> isize
    where (!SomeComparator<Cmp>
           && is_total_ordered<ElemIn<Arr>>
           && is_total_ordered_w<ElemIn<Arr>, PlainT<Key>>)
{
    usize i = 0;
    while (i < arr.len && cmp(arr[i], key)) {
        i++;
    }
    if (i < arr.len && !cmp(arr[i], key) && eq(arr[i], key)) {
        return (isize) i;
    }
    return -i - 1;
}

///
template<
    SomeInlineArray Arr, typename Elm, OrderType Cmp = Lne
>
cons fn insert(Arr& arr, Elm&& elm, Cmp cmp = lne) noexce -> isize
    where is_total_ordered<ElemIn<Arr>>&& same_or_cvref<ElemIn<Arr>, Elm>
{
    if (arr.len == arr.cap) {
        return -1;
    }

    isize i = arr::find(arr, elm, cmp);
    if (i >= 0) {
        return -1;
    }
    i = -i - 1;

    for (isize j = arr.len; j > i; j--) {
        arr[j] = arr[j - 1];
    }

    arr[i] = elm;
    arr.len++;
    return i;
}

// cons fn remove(Inline_Array& arr, KeyIn(arr) key) -> isize {
// }

// =========================================
// Testing
// =========================================

#ifdef CX_TEST
#ifndef CX_TEST_INLINE_ARRAY
#define CX_TEST_INLINE_ARRAY
#include <stdio.h>

fn test_inline_array_compare() -> void
{
    using CX;
    InlineArray<i32, 8> a{};
    push_back(a, 42);
    InlineArray<i32, 6> b{};
    bool x = a > b;
    printf("%d\n", x);
}

#endif
#endif

}       // namespace arr
}       // namespace cx
#endif  // CX_ARR_INLINE_ARRAY_HH
