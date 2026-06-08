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
#include "libcx/mem/allocator.hh"

namespace cx {
inline namespace arr {

template<ZeroInitble E, SomeAllocator A = HeapAllocator, typename S = isize>
struct DynamicArray {
    CX_DEFINE_MEMBER_TYPES(E, S);
    using Alc = A;
    using Self = DynamicArray<E, A, S>;

    Iter ptr{null};
    Size len{0};
    Size cap{0};
    Alc  alc{};

    // TODO: define the operator using forward_with 
    inln cons fn operator[](this auto& arr, isize const i) noexce -> Elem&
    {
        return arr.ptr[i];
    }
    inln cons fn operator[](this auto const& arr, isize const i) noexce
    {
        return arr.ptr[i];
    }

    inln cons fn begin() noexce  -> Iter { return ptr; }
    inln cons fn end() noexce -> Iter { return ptr + len; }
    inln cons fn begin() const noexce -> Kter { return ptr; }
    inln cons fn end() const noexce -> Kter { return ptr + len; }
};

template<typename E> DynamicArray(E) -> DynamicArray<E>;

template<typename E, typename... Es>
DynamicArray(E, Es...) -> DynamicArray<enable_if<bvariand<same_as<E, Es>...>, E>>;

CX_CONCEPT_GEN_TEMPL(
    DynamicArray, is_array, SomeDynamicArray,
    VA_(typename E, SomeAllocator A, typename S), VA_(E, A, S)
);

template<typename T, typename E> predicate is_array_of = false;
template<typename T, typename S, SomeAllocator A> predicate is_array_of<DynamicArray<T, S, A>, T> = true;
template<typename Arr, typename E> concept SomeArrayOf = is_array_of<Arr, E>;

/** Equality operator. **/
cons fn operator==(
    SomeDynamicArray auto const& a, SomeDynamicArray auto const& b
) noexce->bool {
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
operator<=>(SomeDynamicArray auto const& a, SomeDynamicArray auto const& b) noexce->i8
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

inln priv cons fn _rep_ok(SomeDynamicArray auto& arr) noexce -> b32
{
    if ((arr.ptr == null && arr.len != 0) || arr.len < 0 || arr.cap < 0
        || arr.len > arr.cap || !is_power_of_two(arr.cap)
    ) {
        return false;
    }
    return true;
}

cons fn resize(SomeDynamicArray auto& arr, isize new_cap) noexce -> ErrorCode
{
    CX_USING_MEMBER_TYPES_OF(arr);

    if (new_cap < 16) {
        new_cap = 16;
    }
    // assert(is_power_of_two(new_cap)); XXX: waht if i round to the next p2

    isize bytes = size_of(Elem);
    auto [ptr, err] = aligned_resize(
        arr.alc, arr.ptr, new_cap * bytes, arr.cap * bytes, align_of(Elem)
    ) or_return err;

    arr.ptr = Iter(ptr);
    arr.cap = new_cap;
    return null;
}

cons fn resize(SomeDynamicArray auto& arr) noexce -> ErrorCode
{
    if (arr.cap == 0) unlike {
        return resize(arr, 16);
    }
    return resize(arr, arr.cap * 2);
}

// To be used for quick inline variable definitions from temporary values.
template<SomeAllocator A = HeapAllocator, typename S = isize, typename E, typename... Es>
cons fn dynamic_array(
    E&& e, Es&&... es
) noexce -> Res<DynamicArray<PlainT<E>, A, S>, ErrorCode>
    where bvariand<same_as<PlainT<E>, PlainT<Es>>...>
{
    using Elem = PlainT<E>;

    DynamicArray<Elem, A, S> arr{};
    ErrorCode err;
    err = resize(arr, 1 + sizeof...(Es)) or_return {arr, err};
    arr.ptr[0] = forward<E>(e);

    isize i = 1;
    ((arr.ptr[i++] = forward<Es>(es)), ...);

    arr.len = 1 + sizeof...(Es);

    return {arr, null};
}

template<ZeroInitble E, SomeAllocator A = HeapAllocator, typename S = isize>
cons fn make(isize cap, A alc = heap_allocator()) noexce -> Res<DynamicArray<E, A, S>, ErrorCode>
{
    auto arr = DynamicArray<E, A, S>{.ptr{null}, .len{0}, .cap{0}, .alc{alc}};
    auto err = resize(arr, cap) or_return {DynamicArray<E, A, S>{}, err};
    err = init_type<E>(arr.ptr, arr.cap) or_return {DynamicArray<E, A, S>{}, err};
    arr.cap = cap;
    return {arr, null};
}

// Forwarding, do I want to do that?
cons fn push_tail(SomeDynamicArray auto& arr, auto&& elm) noexce -> ErrorCode
    where same_or_cvref<elem_in(arr), declt(elm)>
{
    if (arr.len == arr.cap) unlike {
        auto err = resize(arr) or_return err;
    }
    arr.ptr[arr.len] = forward<declt(elm)>(elm);
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
    return {arr.ptr[arr.len], null};
}

inln cons fn lshift(SomeDynamicArray auto& arr, size_in(arr) off) noexce -> ErrorCode
{
    if (off > arr.len) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.ptr, arr.ptr + off, arr.len - off);
    arr.len -= off;
    return null;
}

inln cons fn rshift(SomeDynamicArray auto& arr, size_in(arr) off) noexce -> ErrorCode
{
    if (arr.len + off >= arr.cap) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.ptr + off, arr.ptr, off);
    return null;
}

template<SomeDynamicArray Arr, typename Size = SizeIn<Arr>>
inln cons fn lshift(Arr& arr, Size beg, Size end, Size off) noexce -> ErrorCode
{
    if (beg > end || end > arr.cap || off > beg) unlike {
        return Invalid_Arg;
    }
    mem_move(arr.ptr + beg - off, arr.ptr + beg, end - beg);
    return null;
}

template<SomeDynamicArray Arr, typename Size = SizeIn<Arr>>
inln cons fn rshift(Arr& arr, Size beg, Size end, Size off) noexce -> bool
{
    if (beg > end || end > arr.cap || off > arr.cap - end) unlike {
        return false;
    }

    mem_move(arr.ptr + beg + off, arr.ptr + beg, end - beg);
    return true;
}

/** 
    Finds the insertion point after the last element ordered before or equal to `key`.
    With duplicates, the insertion point is after the existing equal range.
    @para
    - `arr`: the array to search
    - `key`: the key to compare against
    - `cmp`: the ordering predicate
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
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
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
    - `cmp`: the ordering predicate
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
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
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
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
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
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
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
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) > 16)
{
    if (isize err; arr.len >= arr.cap) {
        err = resize(arr) or_return - 1;
    }
    isize i = -find_last(arr, key, cmp) - 1;
    rshift(arr, i, arr.len, 1);
    arr[i] = key;
    arr.len += 1;
    return i;
}

template<SomeDynamicArray Arr, typename Key, OrderType Cmp = Lne>
cons fn insert(Arr& arr, Key key, Cmp cmp = lne) noexce -> isize
    where (is_total_ordered_w<ElemIn<Arr>, PlainT<Key>> && size_of(Key) <= 16)
{
    if (isize err; arr.len >= arr.cap) {
        err = resize(arr) or_return - 1;
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
