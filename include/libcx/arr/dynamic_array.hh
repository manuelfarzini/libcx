/** @file libcx/arr/dynamic_array.hh **/

#ifndef CX_ARR_DYNAMIC_ARRAY_HH
#define CX_ARR_DYNAMIC_ARRAY_HH

#include "libcx/conf/hal.hh"
#include "libcx/conf/macro.hh"
#include "libcx/trait.hh"
#include "libcx/concept.hh"
#include "libcx/uti/compare.hh"
#include "libcx/uti/iterator.hh"
#include "libcx/uti/members.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/allocator.hh"

namespace cx {
inline namespace arr {

onedef cons isize DYNAMIC_ARRAY_MIN_CAP = 16;

template<PassiveValue E, typename S = isize>
struct DynamicArray
{
    CX_DEFINE_MEMBER_TYPES(E, S);
    using Self = DynamicArray<E, S>;

    Iter       buf;
    Size       len;
    Size       cap;
    Allocator  alc;

    // TODO:(manu) Should I define the operator using forward_with?
    inln cons fn operator[](isize i) noexce { return buf[i]; }
    inln cons fn operator[](isize i) const noexce { return buf[i]; }

    inln cons fn begin() noexce -> Iter { return buf; }
    inln cons fn end() noexce -> Iter { return buf + len; }
    inln cons fn begin() const noexce -> Kter { return buf; }
    inln cons fn end() const noexce -> Kter { return buf + len; }
};

template<typename E> DynamicArray(E) -> DynamicArray<E>;

template<typename E, typename... Es>
DynamicArray(E, Es...) -> DynamicArray<enable_if<(same_as<E, Es> && ...), E>>;

CX_CONCEPT_GEN_TEMPL(
    DynamicArray, is_array, SomeDynamicArray,
    VA_(typename E, typename S), VA_(E, S)
);

template<typename T, typename E> proposition is_array_of = false;
template<typename T, typename S> proposition is_array_of<DynamicArray<T, S>, T> = true;
template<typename Arr, typename E> concept SomeArrayOf = is_array_of<Arr, E>;

/** Equality operator. **/
cons fn
operator==(SomeDynamicArray auto const& a, SomeDynamicArray auto const& b) noexce -> bool
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

/** Comparison operator. **/
cons fn
operator<=>(SomeDynamicArray auto const& a, SomeDynamicArray auto const& b) noexce -> i8
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

// TODO:(manu) To be implemented.
// inln priv cons fn to_string(SomeDynamicArray auto const& arr) noexce -> String;

inln priv cons fn rep_ok(SomeDynamicArray auto const& arr) noexce -> b32
{
    using Elem = ElemIn<decltype(arr)>;

    if (arr.len < 0 || arr.cap < 0 || arr.len > arr.cap) {
        return false;
    }
    if (arr.buf == null) {
        return arr.len == 0 && arr.cap == 0;
    }
    if (arr.alc.call == null || arr.cap < DYNAMIC_ARRAY_MIN_CAP) {
        return false;
    }
    if (uptr(arr.buf) % uptr(align_of(Elem)) != 0) {
        return false;
    }
    return true;
}

/** Allocates raw storage for at least `cap` elements without constructing them. **/
template<PassiveValue E, typename S = isize>
cons fn make(
    isize      cap,
    Allocator  alc    =  heap_allocator(),
    AllocFlag  flags  =  AllocFlag_Zero
) noexce -> Res<DynamicArray<E, S>, ErrorCode> {
    if (cap < 0) {
        return {DynamicArray<E, S>{null, 0, 0, alc}, Invalid_Arg};
    }
    if (cap < DYNAMIC_ARRAY_MIN_CAP) {
        cap = DYNAMIC_ARRAY_MIN_CAP;
    }

    auto [buf, err] = mem_alloc(alc, cap * size_of(E), align_of(E), flags)
        or_return {DynamicArray<E, S>{null, 0, 0, alc}, err};

    auto arr = DynamicArray<E, S>{cast(E*, buf), S(0), S(cap), alc};
    cx_assert(rep_ok(arr));
    return {arr, null};
}

// To be used for quick inline variable definitions from temporary values.
template<typename S = isize, typename E, typename... Es>
cons fn dynamic_array(
    Allocator a, E&& e, Es&&... es
) noexce -> Res<DynamicArray<PlainT<E>, S>, ErrorCode>
    with (PassiveValue<PlainT<E>, PlainT<Es>...> && (same_as<PlainT<E>, PlainT<Es>> && ...))
{
    using Elem = PlainT<E>;

    isize n = 1 + isize(sizeof...(Es));
    auto [arr, err] = make<Elem, S>(n, a, AllocFlag_None)
        or_return {DynamicArray<Elem, S>{null, 0, 0, a}, err};
    

    Elem vals[] { forward<E>(e), forward<Es>(es)... };
    mem_copy(arr.buf, vals, n);
    arr.len = n;

    cx_assert(rep_ok(arr));
    return {arr, null};
}

template<typename S = isize, typename E, typename... Es>
inln cons fn dynamic_array(
    E&& e, Es&&... es
) noexce -> Res<DynamicArray<PlainT<E>, S>, ErrorCode> with
(
    not same_as<PlainT<E>, Allocator> && PassiveValue<PlainT<E>, PlainT<Es>...> &&
        (same_as<PlainT<E>, PlainT<Es>> && ...)
) {
    return dynamic_array<S>(heap_allocator(), forward<E>(e), forward<Es>(es)...);
}

template<SomeDynamicArray Arr>
cons fn reserve(Arr& arr, SizeIn<Arr> new_cap) noexce -> ErrorCode
{
    using Elem = ElemIn<Arr>;
    using Size = SizeIn<Arr>;

    if (new_cap <= arr.cap) {
        return null;
    }
    if (new_cap < DYNAMIC_ARRAY_MIN_CAP) {
        new_cap = DYNAMIC_ARRAY_MIN_CAP;
    }

    Size grown = arr.cap > 0 ? arr.cap * 2 : DYNAMIC_ARRAY_MIN_CAP;
    if (new_cap < grown) {
        new_cap = grown;
    }

    auto [buf, err] = mem_resize(
        arr.alc,
        arr.buf,
        arr.cap * size_of(Elem),
        new_cap * size_of(Elem),
        align_of(Elem),
        AllocFlag_None
    ) or_return err;

    arr.buf = cast(Elem*, buf);
    arr.cap = new_cap;
    cx_assert(rep_ok(arr));
    return null;
}

template<SomeDynamicArray Arr>
cons fn resize(Arr& arr, SizeIn<Arr> new_len) noexce -> ErrorCode
{
    using Elem = ElemIn<Arr>;

    if (new_len < 0) {
        return Invalid_Arg;
    }
    if (new_len > arr.cap) {
        auto err = reserve(arr, new_len) or_return err;
    }
    if (new_len > arr.len) {
        auto err = init_value<Elem>(arr.buf + arr.len, new_len - arr.len) or_return err;
    }

    arr.len = new_len;
    cx_assert(rep_ok(arr));
    return null;
}

template<SomeDynamicArray Arr>
cons fn shrink(Arr& arr) noexce -> ErrorCode
{
    using Elem = ElemIn<Arr>;
    using Size = SizeIn<Arr>;

    Size new_cap = arr.len > DYNAMIC_ARRAY_MIN_CAP ? arr.len : DYNAMIC_ARRAY_MIN_CAP;
    if (new_cap >= arr.cap) {
        return null;
    }

    // Allocators may keep the old block when asked to shrink. The returned
    // storage is still valid for `new_cap`, and ownership remains with `arr`.
    auto [buf, err] = mem_resize(
        arr.alc,
        arr.buf,
        arr.cap * size_of(Elem),
        new_cap * size_of(Elem),
        align_of(Elem),
        AllocFlag_None
    ) or_return err;

    arr.buf = cast(Elem*, buf);
    arr.cap = new_cap;
    cx_assert(rep_ok(arr));
    return null;
}

template<SomeDynamicArray Arr>
cons fn free(Arr& arr) noexce -> ErrorCode
{
    if (arr.buf == null) {
        arr.len = 0;
        arr.cap = 0;
        return null;
    }

    // NOTE:(manu)
    // Allocators without individual frees report an error here; keep the
    // storage owned by `arr` in that case.
    auto [_, err] = mem_free(arr.alc, arr.buf) or_return err;

    arr.buf = null;
    arr.len = 0;
    arr.cap = 0;
    cx_assert(rep_ok(arr));
    return null;
}

// XXX:(manu) Forwarding, do I want to do that?
cons fn push_tail(SomeDynamicArray auto& arr, auto&& elm) noexce -> ErrorCode
    with same_or_cvref<elem_in(arr), declt(elm)>
{
    if (arr.len == arr.cap) unlike {
        auto err = reserve(arr, arr.len + 1) or_return err;
    }
    arr.buf[arr.len] = forward<declt(elm)>(elm);
    arr.len++;
    return null;
}

// Now always returns a copy
inln cons fn pop_back(SomeDynamicArray auto& arr) noexce -> Res<elem_in(arr), ErrorCode>
{
    if (arr.len == 0) unlike {
        return {null, Operation_Fail};
    }
    arr.len--;
    return {arr.buf[arr.len], null};
}

inln cons fn lshift(SomeDynamicArray auto& arr, size_in(arr) off) noexce -> ErrorCode
{
    if (off > arr.len) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.buf, arr.buf + off, arr.len - off);
    arr.len -= off;
    return null;
}

inln cons fn rshift(SomeDynamicArray auto& arr, size_in(arr) off) noexce -> ErrorCode
{
    if (arr.len + off >= arr.cap) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.buf + off, arr.buf, off);
    return null;
}

template<SomeDynamicArray Arr, typename Size = SizeIn<Arr>>
inln cons fn lshift(Arr& arr, Size beg, Size end, Size off) noexce -> ErrorCode
{
    if (beg > end || end > arr.cap || off > beg) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.buf + beg - off, arr.buf + beg, end - beg);
    return null;
}

template<SomeDynamicArray Arr, typename Size = SizeIn<Arr>>
inln cons fn rshift(Arr& arr, Size beg, Size end, Size off) noexce -> bool
{
    if (beg > end || end > arr.cap || off > arr.cap - end) unlike {
        return false;
    }

    mem_move(arr.buf + beg + off, arr.buf + beg, end - beg);
    return true;
}

/** 
    Finds the insertion point after the last element ordered before or equal to `key`.
    With duplicates, the insertion point is after the existing equal range.
    @para
    - `arr`: the array to search
    - `key`: the key to compare against
    - `cmp`: the ordering proposition
    @ret
    - the encoded insertion point `-i - 1`, where `i` is after the last matching key
    @pre
    - `arr` is ordered according to `cmp`
    - `arr` has at least one element
    @nota
    - This overload is for large keys, passed by reference.
**/
template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
inln cons fn find_last(Arr const& arr, Key const& key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
{
    if (arr.len <= 0) unlike {
        return -1; // XXX:
    }
    isize i = 0;
    isize mid = arr.len/2;
    if (!cmp(key, arr[mid])) {
        i = mid + 1;
    }
    for (; i < arr.len && !cmp(key, arr[i]);) {
        i = i + 1;
    }
    return -i - 1;
}

/** 
    Finds the insertion point after the last element ordered before or equal to `key`.
    With duplicates, the insertion point is after the existing equal range.
    @para
    - `arr`: the array to search
    - `key`: the key to compare against
    - `cmp`: the ordering proposition
    @ret
    - the encoded insertion point `-i - 1`, where `i` is after the last matching key
    @pre
    - `arr` is ordered according to `cmp`
    - `arr` has at least one element
    @nota
    - This overload is for small keys, passed by value.
**/
template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
inln cons fn find_last(Arr const& arr, Key key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
{
    if (arr.len <= 0) unlike {
        return -1; // XXX:
    }
    isize i = 0;
    isize mid = arr.len/2;
    if (!cmp(key, arr[mid])) {
        i = mid + 1;
    }
    for (; i < arr.len && !cmp(key, arr[i]);) {
        i = i + 1;
    }
    return -i - 1;
}

template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
inln cons fn find_first(Arr const& arr, Key const& key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
{
    if (arr.len <= 0) unlike {
        return -1;
    }
    isize i = 0;
    isize mid = arr.len / 2;
    if (cmp(arr[mid], key)) {
        i = mid + 1;
    }
    for (; i < arr.len && cmp(arr[i], key);) {
        i = i + 1;
    }
    if (i < arr.len && eq(arr[i], key)) {
        return i;
    }
    return -i - 1;
}

template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
inln cons fn find_first(Arr const& arr, Key key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
{
    if (arr.len <= 0) unlike {
        return -1;
    }
    isize i = 0;
    isize mid = arr.len / 2;
    if (cmp(arr[mid], key)) {
        i = mid + 1;
    }
    for (; i < arr.len && cmp(arr[i], key);) {
        i = i + 1;
    }
    if (i < arr.len && eq(arr[i], key)) {
        return i;
    }
    return -i - 1;
}

template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
cons fn insert(Arr& arr, Key const& key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
{
    if (isize err; arr.len >= arr.cap) {
        err = reserve(arr, arr.len + 1) or_return - 1;
    }
    isize i = -find_last(arr, key, cmp) - 1;
    rshift(arr, i, arr.len, 1);
    arr[i] = key;
    arr.len += 1;
    return i;
}

template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
cons fn insert(Arr& arr, Key key, Cmp cmp = lne) noexce -> isize
    with (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
{
    if (isize err; arr.len >= arr.cap) {
        err = reserve(arr, arr.len + 1) or_return - 1;
    }
    isize i = -find_last(arr, key, cmp) - 1;
    rshift(arr, i, arr.len, 1);
    arr[i] = key;
    arr.len += 1;
    return i;
}

////////////////////////////////////////////
// Testing

// #define CX_TEST_DYNARR 1
#if CX_TEST_DYNARR

CX_TEST_DEFINE(dynamic_array_basic) {
    auto [arr, err] = make<isize>(16);
    if (err) {
        printf("%d\n", err);
        return;
    }
    for (isize i = 0; i < 10; i++) {
        push_tail(arr, i);
    }
    puts("\npre shift\n");
    for (auto a : arr) {
        printf("%ld ", a);
    }
    lshift(arr, 2);
    puts("\n\npost lshift\n");
    for (auto a : arr) {
        printf("%ld ", a);
    }
    rshift(arr, 2);
    puts("\n\npost rshift\n");
    for (auto a : arr) {
        printf("%ld ", a);
    }
    puts("");
}

#endif

}  // namespace arr
}  // namespace cx

// NOTE: facendo partire la capacita' da un minimo, come 16 o 16, risparmio le prime
// reallocazioni, inoltre posso anche fare in modo di taggare il puntatore per avere
// la capacita' in multipli di 2 usando pochi bit, cosi' entro una certa lunghezza e
// capacita' e' addirittura possibile taggare nel puntatore entrambe!


#endif  // CX_ARR_DYNAMIC_ARRAY_HH
