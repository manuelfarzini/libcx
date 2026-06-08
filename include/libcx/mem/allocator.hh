/** @file mem/allocator.hh **/

#ifndef CX_MEM_ALLOCATOR_HH
#define CX_MEM_ALLOCATOR_HH

#include "libcx/conf/type.hh"
#include "libcx/conf/prelude.hh"
#include "libcx/traits/types.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/pointer.hh"

////////////////////////////////////////////
// Prelude

namespace cx {
inline namespace mem {

onedef cons u32 AllocFlags_None    = 0;
onedef cons u32 AllocFlags_Zero    = BIT<0>;
onedef cons u32 AllocFlags_Default = AllocFlags_Zero;

///////////////////////////////////////////
// Allocator contract

template<typename Alc>
concept SomeAllocator = requires(
    Alc&       alc,
    mutaptr    old_ptr,
    isize      old_size,
    isize      new_size,
    isize      old_align,
    isize      new_align,
    u32        flags
) {
    { aligned_alloc(alc, new_size, new_align, flags) }
    noexce -> SameAs<Res<mutaptr, ErrorCode>>;

    { aligned_resize(alc, old_ptr, new_size, old_size, new_align, old_align, flags) }
    noexce -> SameAs<Res<mutaptr, ErrorCode>>;

    { aligned_free(alc, old_ptr) } noexce -> SameAs<ErrorCode>;
};

////////////////////////////////////////////
// Allocator public API

// Static allocator interface.

#define ALIGNED_ALLOC(name, Alc)              \
    cons fn name(                             \
        Alc&     alc,                         \
        isize    size,                        \
        isize    align  =  DEF_ALIGN,         \
        u32      flags  =  AllocFlags_Default \
    ) noexce -> Res<mutaptr, ErrorCode>

#define ALIGNED_RESIZE(name, Alc)                   \
    cons fn name(                                   \
        Alc&       alc,                             \
        mutaptr    old_ptr,                         \
        isize      old_size,                        \
        isize      new_size,                        \
        isize      old_align  =  DEF_ALIGN,         \
        isize      new_align  =  DEF_ALIGN,         \
        u32        flags      =  AllocFlags_Default \
    ) noexce -> Res<mutaptr, ErrorCode>

#define ALIGNED_FREE(name, Alc) \
    cons fn name(               \
        Alc& alc, mutaptr ptr   \
    ) noexce -> ErrorCode

// Runtime allocator interface.

#define ALIGNED_ALLOC_VIEW(name)                \
    fn name(                                    \
        mutaptr    alc,                         \
        isize      size,                        \
        isize      align  =  DEF_ALIGN,         \
        u32        flags  =  AllocFlags_Default \
    ) noexce -> Res<mutaptr, ErrorCode>

#define ALIGNED_RESIZE_VIEW(name)                   \
    fn name(                                        \
        mutaptr    alc,                             \
        mutaptr    old_ptr,                         \
        isize      old_size,                        \
        isize      new_size,                        \
        isize      old_align  =  DEF_ALIGN,         \
        isize      new_align  =  DEF_ALIGN,         \
        u32        flags      =  AllocFlags_Default \
    ) noexce -> Res<mutaptr, ErrorCode>

#define ALIGNED_FREE_VIEW(name)  \
    fn name(                     \
        mutaptr alc, mutaptr ptr \
    ) noexce -> ErrorCode

////////////////////////////////////////////
// Allocator view runtime adapter

// Allocator view definition.

using AlignedFreeView = func(
    mutaptr data, mutaptr ptr
) noexce -> ErrorCode;

using AlignedAllocView = func( 
    mutaptr    data,
    isize     size,
    isize     align,
    u32       flags
) noexce -> Res<mutaptr, ErrorCode>;

using AlignedResizeView = func(
    mutaptr    data, 
    mutaptr    old_ptr,
    isize     new_size,
    isize     old_size,
    isize     new_align,
    isize     old_align,
    u32       flags
) noexce -> Res<mutaptr, ErrorCode>;

struct AllocatorView
{
    mutaptr              data;
    AlignedAllocView*   alloc;
    AlignedResizeView*  resize;
    AlignedFreeView*    free;
};

template<SomeAllocator Alc>
ALIGNED_ALLOC_VIEW(aligned_alloc_view)
{
    return aligned_alloc(cast(Alc*, alc)->data, size, align, flags);
}

template<SomeAllocator Alc>
ALIGNED_RESIZE_VIEW(aligned_resize_view)
{
    return aligned_resize(cast(Alc*, alc)->data, old_ptr, old_size, new_size, old_align, new_align, flags);
}

template<SomeAllocator Alc>
ALIGNED_FREE_VIEW(aligned_free_view)
{
    return aligned_free(cast(Alc*, alc)->data, ptr);
}

// Allocator view dispatcher.

ALIGNED_ALLOC(aligned_alloc, AllocatorView)
{
    return alc.alloc(alc.data, size, align, flags);
}

ALIGNED_RESIZE(aligned_resize, AllocatorView)
{
    return alc.resize(alc.data, old_ptr, new_size, old_size, new_align, old_align, flags);
}

ALIGNED_FREE(aligned_free, AllocatorView)
{
    return alc.free(alc.data, ptr);
}

template<SomeAllocator Alc>
nodisc fn allocator_view(Alc& alc) noexce -> AllocatorView
{
    return AllocatorView{
        mutaptr(&alc),
        aligned_alloc_view<Alc>,
        aligned_resize_view<Alc>,
        aligned_free_view<Alc>
    };
}

///////////////////////////////////////////
// Heap allocator definition

struct HeapAllocator {};

glob HeapAllocator HEAP_ALLOCATOR{};
comp fn heap_allocator() noexce -> HeapAllocator& { return HEAP_ALLOCATOR; }

cons fn heap_alloc(isize size, b32 flags = AllocFlags_Default) noexce -> mutaptr
{
    if (size <= 0) {
        return null;
    }

    if (flags & AllocFlags_Zero) {
        return ::calloc(1, size);
    } else {
        return ::malloc(size);
    }
}

cons fn heap_resize(mutaptr ptr, isize new_size) noexce -> mutaptr
{
    return ::realloc(ptr, new_size);
}

cons fn heap_free(mutaptr ptr) noexce -> void
{
    return ::free(ptr);
}

ALIGNED_FREE(aligned_free, HeapAllocator)
{
    // XXX:(manu)
    // - Should return void? Should passing a null pointer be an error?
    cx_unused(alc);
    if (ptr != null) like {
        ::free(cast(void** ,ptr)[-1]);
    }
    return null;
}

ALIGNED_ALLOC(aligned_alloc, HeapAllocator) {
    // XXX:(manu)
    // - Should I impose bytes % align == 0 like std::aligned_alloc?
    // - What if size < 0? Should I implement some logic?
    cx_unused(alc);
    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    if (align < DEF_ALIGN) {
        align = DEF_ALIGN;
    }
    // assert(is_power_of_two(align) && "`align` must be a power of 2");

    isize space = PTR_SIZE + (align - 1) + size;
    mutaptr alloced_mem = heap_alloc(space, flags);
    if (alloced_mem == null) {
        return {null, Alloc_Exhausted};
    }

    mutaptr aligned_mem = align_up(ptr_add(alloced_mem, PTR_SIZE), align);
    (cast(void** ,aligned_mem))[-1] = alloced_mem;

    if (flags & AllocFlags_Zero) {
        mem_zero(aligned_mem, size);
    }

    return {aligned_mem, none};
}


/**
    @ret
    - `[new_ptr, none]` on success
    - otherwise `panic`
**/
ALIGNED_RESIZE(aligned_resize, HeapAllocator) {
    if (new_size <= 0) {
        aligned_free(alc, old_ptr);
        return {null, null};
    }

    if (old_ptr == null) {
        return aligned_alloc(alc, new_size, new_align, flags);
    }

    if (new_align < DEF_ALIGN) {
        new_align = DEF_ALIGN;
    }
    // assert(is_power_of_two(new_align) && "`new_align` must be a power of 2");

    mutaptr alloced_mem = null;
    mutaptr aligned_mem = null;
    b32 force_copy = old_align > DEF_ALIGN || new_align > DEF_ALIGN;
    isize space = PTR_SIZE + (new_align - 1) + new_size;

    if (force_copy) {
        alloced_mem = heap_alloc(space, AllocFlags_None);
        if (alloced_mem == null) {
            return {null, Alloc_Exhausted};
        }
        aligned_mem = align_up(ptr_add(alloced_mem, PTR_SIZE), new_align);
        (cast(void** ,aligned_mem))[-1] = alloced_mem;
        mem_copy(aligned_mem, old_ptr, cx_min2(old_size, new_size));
        aligned_free(alc, old_ptr);

    } else {
        mutaptr base_ptr = cast(void** ,old_ptr)[-1];
        alloced_mem = heap_resize(base_ptr, space);
        if (alloced_mem == null) {
            return {null, Alloc_Exhausted};
        }
        aligned_mem = align_up(ptr_add(alloced_mem, PTR_SIZE), new_align);
        (cast(void** ,aligned_mem))[-1] = alloced_mem;
    }

    if ((flags & AllocFlags_Zero) && (new_size > old_size)) {
        // actually using std malloc and realloc, not os specific primitives, so
        // the new_region should be zeroed in windows too
        mutaptr new_region = ptr_add(aligned_mem, old_size);
        // if not dealing with vm paging plain mem_zero is faster than conditional
        mem_zero(new_region, new_size - old_size);
    }

    return {aligned_mem, none};
}

template<ZeroInitble T>
cons fn aligned_alloc_type(HeapAllocator alc, isize num) -> Res<T*, ErrorCode>
{
    auto [ptr, err] = aligned_alloc(alc, num * size_of(T), align_of(T)) or_return {null, err};
    return {cast(T*, ptr), err};
}

////////////////////////////////////////////
// Arena allocator definition

struct Arena {
    byteptr  data      {};
    isize    data_cap  {};
    isize    curr_off  {};
    isize    prev_off  {};
};

struct ArenaAllocator { Arena& arena; };
static_assert(size_of(ArenaAllocator) == PTR_SIZE);

cons fn arena_init(Arena& arena, isize cap) noexce -> ErrorCode
{
    auto [ptr, err] = aligned_alloc(heap_allocator(), cap, DEF_ALIGN, AllocFlags_Default)
        or_return err;
    arena.data = byteptr(ptr);
    arena.data_cap = cap;
    arena.curr_off = 0;
    arena.prev_off = 0;
    return Error_None;
}

cons fn arena_allocator(Arena& arena) -> ArenaAllocator
{
    return ArenaAllocator{arena};
}

ALIGNED_FREE(aligned_free, ArenaAllocator)
{
    cx_unused(alc);
    cx_unused(ptr);
    return Alloc_Invalid_Mode;
}

// NOTE(manu) Actually super basic version, should implement virtual mem allocator too.
ALIGNED_ALLOC(aligned_alloc, ArenaAllocator)
{
    byteptr old_ptr = ptr_add(alc.arena.data, alc.arena.curr_off);
    isize off = align_up(isize(old_ptr), align) - isize(alc.arena.data);
    if (off + size <= alc.arena.data_cap) {
        mutaptr ptr = ptr_add(alc.arena.data, off);
        alc.arena.prev_off = off;
        alc.arena.curr_off = off + size;
        if (flags & AllocFlags_Zero) {
            mem_zero(ptr, size);
        }
        return {ptr, none};
    }
    return {null, Alloc_Exhausted};
}

static_assert(SomeAllocator<HeapAllocator>);
// static_assert(SomeAllocator<ArenaAllocator>);
// static_assert(SomeAllocator<AllocatorView>);

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_ALLOCATOR_HH
