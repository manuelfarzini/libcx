/** @file test/test_dynamic_array.cc **/

#include <catch/catch_amalgamated.hh>

#include "libcx/arr/dynamic_array.hh"

using namespace cx;

////////////////////////////////////////////
// Test allocator

struct TestAlloc
{
    isize      allocs{};
    isize      frees{};
    isize      resizes{};
    AllocFlag  alloc_flags{};
    AllocFlag  resize_flags{};
    b32        fail_alloc{};
    b32        fail_resize{};
    b32        fail_free{};
    b32        shrink_move{};
};

fn test_alloc_proc(
    mutaptr    data,
    AllocMode  mode,
    isize      size,
    isize      align,
    mutaptr    old_ptr,
    isize      old_size,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    auto& st = *cast(TestAlloc*, data);
    switch (mode) {
    case AllocMode_Alloc:
        st.allocs++;
        st.alloc_flags = flags;
        if (st.fail_alloc) {
            return {null, Alloc_Exhausted};
        }
        return heap_alloc_aligned(size, align, flags);
    case AllocMode_Free:
        st.frees++;
        if (st.fail_free) {
            return {null, Alloc_InvalidMode};
        }
        heap_free_aligned(old_ptr);
        return {null, Error_None};
    case AllocMode_Resize:
        st.resizes++;
        st.resize_flags = flags;
        if (st.fail_resize) {
            return {old_ptr, Alloc_Exhausted};
        }
        if (st.shrink_move && old_ptr != null && size < old_size) {
            auto [new_ptr, alloc_err] = heap_alloc_aligned(size, align, AllocFlag_None);
            if (alloc_err) {
                return {old_ptr, alloc_err};
            }
            mem_copy(new_ptr, old_ptr, size);
            heap_free_aligned(old_ptr);
            return {new_ptr, Error_None};
        }
        return heap_resize_aligned(old_ptr, old_size, size, align, flags);
    case AllocMode_FreeAll:
        return {null, Alloc_InvalidMode};
    }
    cx_unreachable;
}

inln fn test_allocator(TestAlloc& st) noexce -> Allocator
{
    return {test_alloc_proc, mutaptr(&st)};
}

////////////////////////////////////////////
// Test element types

static isize g_default_constructed = 0;

struct Counted
{
    isize v;

    Counted() : v(7) { g_default_constructed++; }
};

struct NonZero
{
    isize v = 42;
};

struct alignas(64) OverAligned
{
    u64 v = 0;
};

static_assert(PassiveValue<Counted>);
static_assert(PassiveValue<NonZero>);
static_assert(PassiveValue<OverAligned>);

////////////////////////////////////////////
// Allocation

TEST_CASE("dynamic_array make allocates at least the minimum capacity")
{
    auto [arr, err] = make<isize>(1);
    REQUIRE(err == Error_None);
    REQUIRE(arr.buf != null);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(free(arr) == Error_None);
    REQUIRE(arr.buf == null);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == 0);

    auto [big, big_err] = make<isize>(64);
    REQUIRE(big_err == Error_None);
    REQUIRE(big.cap == 64);
    REQUIRE(free(big) == Error_None);
}

TEST_CASE("dynamic_array representation invariants")
{
    auto [arr, err] = make<isize>(DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(err == Error_None);
    REQUIRE(_rep_ok(arr));

    arr.len = arr.cap + 1;
    REQUIRE_FALSE(_rep_ok(arr));
    arr.len = 0;

    arr.cap = DYNAMIC_ARRAY_MIN_CAP - 1;
    REQUIRE_FALSE(_rep_ok(arr));
    arr.cap = DYNAMIC_ARRAY_MIN_CAP;

    REQUIRE(free(arr) == Error_None);
    REQUIRE(_rep_ok(arr));
}

TEST_CASE("dynamic_array make zeroes by default and honors AllocFlag_None")
{
    TestAlloc st;
    auto alloc = test_allocator(st);

    auto [zeroed, zero_err] = make<u64>(16, alloc);
    REQUIRE(zero_err == Error_None);
    REQUIRE(st.alloc_flags == AllocFlag_Zero);
    for (isize i = 0; i < zeroed.cap; i++) {
        REQUIRE(zeroed.buf[i] == 0);
    }
    REQUIRE(free(zeroed) == Error_None);

    auto [unzeroed, unzeroed_err] = make<u64>(16, alloc, AllocFlag_None);
    REQUIRE(unzeroed_err == Error_None);
    REQUIRE(st.alloc_flags == AllocFlag_None);
    REQUIRE(free(unzeroed) == Error_None);
}

TEST_CASE("dynamic_array make respects the element alignment")
{
    auto [arr, err] = make<OverAligned>(1);
    REQUIRE(err == Error_None);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(uptr(arr.buf) % uptr(align_of(OverAligned)) == 0);

    REQUIRE(resize(arr, 3) == Error_None);
    for (isize i = 0; i < 3; i++) {
        REQUIRE(arr.buf[i].v == 0);
    }

    REQUIRE(reserve(arr, 100) == Error_None);
    REQUIRE(uptr(arr.buf) % uptr(align_of(OverAligned)) == 0);

    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(uptr(arr.buf) % uptr(align_of(OverAligned)) == 0);

    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array make does not construct or value-initialize elements")
{
    g_default_constructed = 0;

    auto [arr, err] = make<Counted>(DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(err == Error_None);
    REQUIRE(g_default_constructed == 0);

    REQUIRE(resize(arr, 3) == Error_None);
    REQUIRE(g_default_constructed == 3);
    for (isize i = 0; i < 3; i++) {
        REQUIRE(arr.buf[i].v == 7);
    }

    REQUIRE(free(arr) == Error_None);

    g_default_constructed = 0;
    auto [grown, grown_err] = make<Counted>(DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(grown_err == Error_None);
    REQUIRE(reserve(grown, 100) == Error_None);
    REQUIRE(g_default_constructed == 0);
    REQUIRE(grown.len == 0);
    REQUIRE(free(grown) == Error_None);
}

////////////////////////////////////////////
// Factories

TEST_CASE("dynamic_array constructs values directly")
{
    auto [arr, err] = dynamic_array(10, 20, 30);
    REQUIRE(err == Error_None);
    REQUIRE(arr.len == 3);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(arr.buf[0] == 10);
    REQUIRE(arr.buf[1] == 20);
    REQUIRE(arr.buf[2] == 30);
    REQUIRE(free(arr) == Error_None);

    TestAlloc st;
    auto [with_alc, alc_err] = dynamic_array<isize>(test_allocator(st), 4, 5, 6, 7);
    REQUIRE(alc_err == Error_None);
    REQUIRE(with_alc.len == 4);
    REQUIRE(with_alc.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(st.alloc_flags == AllocFlag_None);
    REQUIRE(with_alc.buf[0] == 4);
    REQUIRE(with_alc.buf[1] == 5);
    REQUIRE(with_alc.buf[2] == 6);
    REQUIRE(with_alc.buf[3] == 7);
    REQUIRE(free(with_alc) == Error_None);
}

TEST_CASE("dynamic_array factory grows beyond the minimum capacity")
{
    auto [arr, err] = dynamic_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                    13, 14, 15, 16, 17);
    REQUIRE(err == Error_None);
    REQUIRE(arr.len == 17);
    REQUIRE(arr.cap == 17);
    for (isize i = 0; i < arr.len; i++) {
        REQUIRE(arr.buf[i] == i + 1);
    }
    REQUIRE(free(arr) == Error_None);
}

////////////////////////////////////////////
// Growth

TEST_CASE("dynamic_array reserve preserves the logical length and values")
{
    TestAlloc st;
    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);

    isize old_cap = arr.cap;
    REQUIRE(reserve(arr, 100) == Error_None);
    REQUIRE(arr.len == 3);
    REQUIRE(arr.cap >= 100);
    REQUIRE(arr.buf[0] == 1);
    REQUIRE(arr.buf[1] == 2);
    REQUIRE(arr.buf[2] == 3);
    REQUIRE(st.resize_flags == AllocFlag_None);

    auto old_buf = arr.buf;
    isize grown_cap = arr.cap;
    isize resize_calls = st.resizes;
    REQUIRE(reserve(arr, old_cap) == Error_None);
    REQUIRE(st.resizes == resize_calls);
    REQUIRE(arr.buf == old_buf);
    REQUIRE(arr.cap == grown_cap);
    REQUIRE(arr.len == 3);

    REQUIRE(free(arr) == Error_None);
}

////////////////////////////////////////////
// Resizing

TEST_CASE("dynamic_array resize value-initializes new elements")
{
    auto [arr, err] = make<NonZero>(4, heap_allocator(), AllocFlag_None);
    REQUIRE(err == Error_None);
    REQUIRE(arr.len == 0);

    REQUIRE(resize(arr, 4) == Error_None);
    REQUIRE(arr.len == 4);
    for (isize i = 0; i < arr.len; i++) {
        REQUIRE(arr.buf[i].v == 42);
    }

    REQUIRE(resize(arr, 6) == Error_None);
    REQUIRE(arr.len == 6);
    for (isize i = 0; i < arr.len; i++) {
        REQUIRE(arr.buf[i].v == 42);
    }

    REQUIRE(resize(arr, 2) == Error_None);
    REQUIRE(arr.len == 2);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);

    REQUIRE(resize(arr, 5) == Error_None);
    REQUIRE(arr.len == 5);
    for (isize i = 0; i < arr.len; i++) {
        REQUIRE(arr.buf[i].v == 42);
    }

    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array resize grows and preserves existing elements")
{
    auto [arr, err] = dynamic_array(10, 20, 30);
    REQUIRE(err == Error_None);

    REQUIRE(resize(arr, 40) == Error_None);
    REQUIRE(arr.len == 40);
    REQUIRE(arr.cap >= 40);
    REQUIRE(arr.buf[0] == 10);
    REQUIRE(arr.buf[1] == 20);
    REQUIRE(arr.buf[2] == 30);
    for (isize i = 3; i < arr.len; i++) {
        REQUIRE(arr.buf[i] == 0);
    }

    REQUIRE(resize(arr, 3) == Error_None);
    REQUIRE(arr.len == 3);
    REQUIRE(arr.cap >= 40);
    REQUIRE(arr.buf[0] == 10);
    REQUIRE(arr.buf[1] == 20);
    REQUIRE(arr.buf[2] == 30);

    REQUIRE(free(arr) == Error_None);
}

////////////////////////////////////////////
// Shrinking

TEST_CASE("dynamic_array shrink reduces capacity to max(len, minimum)")
{
    auto [arr, err] = dynamic_array(1, 2, 3);
    REQUIRE(err == Error_None);
    REQUIRE(reserve(arr, 100) == Error_None);
    REQUIRE(arr.cap >= 100);

    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(arr.len == 3);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(arr.buf[0] == 1);
    REQUIRE(arr.buf[1] == 2);
    REQUIRE(arr.buf[2] == 3);

    REQUIRE(resize(arr, 40) == Error_None);
    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(arr.len == 40);
    REQUIRE(arr.cap == 40);
    REQUIRE(arr.buf[0] == 1);
    REQUIRE(arr.buf[1] == 2);
    REQUIRE(arr.buf[2] == 3);

    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array shrink updates a relocated allocation")
{
    TestAlloc st;
    st.shrink_move = true;

    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);
    REQUIRE(reserve(arr, 100) == Error_None);

    auto old_buf = arr.buf;
    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(arr.len == 3);
    REQUIRE(arr.buf != old_buf);
    REQUIRE(arr.buf[0] == 1);
    REQUIRE(arr.buf[1] == 2);
    REQUIRE(arr.buf[2] == 3);

    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array shrink keeps the minimum capacity when empty")
{
    auto [arr, err] = make<isize>(64);
    REQUIRE(err == Error_None);
    REQUIRE(arr.cap == 64);

    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);

    REQUIRE(shrink(arr) == Error_None);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);

    REQUIRE(free(arr) == Error_None);
}

////////////////////////////////////////////
// Deallocation

TEST_CASE("dynamic_array free releases storage and resets the array")
{
    TestAlloc st;
    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);

    REQUIRE(free(arr) == Error_None);
    REQUIRE(st.frees == 1);
    REQUIRE(arr.buf == null);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == 0);

    REQUIRE(free(arr) == Error_None);
    REQUIRE(st.frees == 1);
}

TEST_CASE("dynamic_array free keeps ownership when the allocator cannot free")
{
    Arena arena;
    REQUIRE(arena_init(arena, 4096) == Error_None);

    auto [arr, err] = make<isize>(DYNAMIC_ARRAY_MIN_CAP, arena_allocator(arena));
    REQUIRE(err == Error_None);
    REQUIRE(arr.buf != null);

    REQUIRE(free(arr) == Alloc_InvalidMode);
    REQUIRE(arr.buf != null);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == DYNAMIC_ARRAY_MIN_CAP);

    REQUIRE(arena_deinit(arena) == Error_None);
}

TEST_CASE("dynamic_array shrink respects allocators without individual frees")
{
    Arena arena;
    REQUIRE(arena_init(arena, 4096) == Error_None);

    auto [first, first_err] = make<isize>(64, arena_allocator(arena));
    REQUIRE(first_err == Error_None);
    auto [second, second_err] = make<isize>(DYNAMIC_ARRAY_MIN_CAP, arena_allocator(arena));
    REQUIRE(second_err == Error_None);

    // The first allocation is not the most recent arena allocation. The arena
    // reports success but keeps the backing region; the capacity remains valid.
    REQUIRE(shrink(first) == Error_None);
    REQUIRE(first.len == 0);
    REQUIRE(first.cap == DYNAMIC_ARRAY_MIN_CAP);
    REQUIRE(first.buf != null);

    REQUIRE(free(first) == Alloc_InvalidMode);
    REQUIRE(first.buf != null);
    REQUIRE(free(second) == Alloc_InvalidMode);
    REQUIRE(second.buf != null);

    REQUIRE(arena_deinit(arena) == Error_None);
}

////////////////////////////////////////////
// Failure handling

TEST_CASE("dynamic_array make propagates allocation failures")
{
    TestAlloc st;
    st.fail_alloc = true;

    auto [arr, err] = make<isize>(DYNAMIC_ARRAY_MIN_CAP, test_allocator(st));
    REQUIRE(err == Alloc_Exhausted);
    REQUIRE(arr.buf == null);
    REQUIRE(arr.len == 0);
    REQUIRE(arr.cap == 0);

    auto [neg, neg_err] = make<isize>(-1);
    REQUIRE(neg_err == Invalid_Arg);
    REQUIRE(neg.buf == null);
    REQUIRE(neg.cap == 0);
}

TEST_CASE("dynamic_array reserve failure preserves the original array")
{
    TestAlloc st;
    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);

    auto old_buf = arr.buf;
    isize old_cap = arr.cap;
    isize old_len = arr.len;

    st.fail_resize = true;
    REQUIRE(reserve(arr, 1000) == Alloc_Exhausted);
    REQUIRE(arr.buf == old_buf);
    REQUIRE(arr.cap == old_cap);
    REQUIRE(arr.len == old_len);
    REQUIRE(arr.buf[0] == 1);
    REQUIRE(arr.buf[1] == 2);
    REQUIRE(arr.buf[2] == 3);

    REQUIRE(resize(arr, 1000) == Alloc_Exhausted);
    REQUIRE(arr.buf == old_buf);
    REQUIRE(arr.cap == old_cap);
    REQUIRE(arr.len == old_len);

    st.fail_resize = false;
    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array shrink failure preserves the original array")
{
    TestAlloc st;
    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);
    REQUIRE(reserve(arr, 100) == Error_None);

    auto old_buf = arr.buf;
    isize old_cap = arr.cap;
    isize old_len = arr.len;

    st.fail_resize = true;
    REQUIRE(shrink(arr) == Alloc_Exhausted);
    REQUIRE(arr.buf == old_buf);
    REQUIRE(arr.cap == old_cap);
    REQUIRE(arr.len == old_len);

    st.fail_resize = false;
    REQUIRE(free(arr) == Error_None);
}

TEST_CASE("dynamic_array free failure preserves ownership")
{
    TestAlloc st;
    auto [arr, err] = dynamic_array<isize>(test_allocator(st), 1, 2, 3);
    REQUIRE(err == Error_None);

    auto old_buf = arr.buf;
    isize old_cap = arr.cap;
    isize old_len = arr.len;

    st.fail_free = true;
    REQUIRE(free(arr) == Alloc_InvalidMode);
    REQUIRE(arr.buf == old_buf);
    REQUIRE(arr.cap == old_cap);
    REQUIRE(arr.len == old_len);

    st.fail_free = false;
    REQUIRE(free(arr) == Error_None);
    REQUIRE(arr.buf == null);
}
