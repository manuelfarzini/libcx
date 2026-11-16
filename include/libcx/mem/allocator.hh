/** @file libcx/mem/allocator.hh **/

#ifndef CX_MEM_ALLOCATOR_HH
#define CX_MEM_ALLOCATOR_HH

#include "libcx/conf/macro.hh"
#include "libcx/conf/prelude.hh"
#include "libcx/conf/type.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/heap.hh"

namespace cx {
inline namespace mem {

////////////////////////////////////////////
// Allocator interface

using AllocFlag = u32;

onedef cons AllocFlag AllocFlag_None    = 0;
onedef cons AllocFlag AllocFlag_Zero    = BIT<0>;
onedef cons AllocFlag AllocFlag_Default = AllocFlag_Zero;

enum AllocMode : u8
{
    AllocMode_Alloc,
    AllocMode_Free,
    AllocMode_Resize,
    AllocMode_FreeAll,
};

using AllocFunc = func (*) (
    mutaptr    data,
    AllocMode  mode,
    isize      size,
    isize      align,
    mutaptr    old_ptr,
    isize      old_size,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode>;

struct Allocator
{
    AllocFunc call;
    mutaptr   data;
};

inln fn mem_alloc(
    Allocator  a,
    isize      size,
    isize      align  =  DEF_ALIGN,
    AllocFlag  flags  =  AllocFlag_Default
) noexce -> Res<mutaptr, ErrorCode> {
    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    return a.call(a.data, AllocMode_Alloc, size, align, null, 0, flags);
}

inln fn mem_free(Allocator a, mutaptr ptr) noexce -> Res<mutaptr, ErrorCode>
{
    return a.call(a.data, AllocMode_Free, 0, 0, ptr, 0, AllocFlag_None);
}

inln fn mem_resize(
    Allocator  a,
    mutaptr    ptr,
    isize      old_size,
    isize      new_size,
    isize      align     =  DEF_ALIGN,
    AllocFlag  flags     =  AllocFlag_Default
) noexce -> Res<mutaptr, ErrorCode> {
    if (ptr == null) {
        return mem_alloc(a, new_size, align, flags);
    }
    return a.call(a.data, AllocMode_Resize, new_size, align, ptr, old_size, flags);
}

inln fn mem_free_all(Allocator a) noexce -> Res<mutaptr, ErrorCode>
{
    return a.call(a.data, AllocMode_FreeAll, 0, 0, null, 0, AllocFlag_None);
}

inln fn mem_free_size(Allocator a, mutaptr ptr, isize size) noexce -> Res<mutaptr, ErrorCode>
{
    // TODO:(manu) Integrate functionality.
    cx_unused(size);
    return mem_free(a, ptr);
}

////////////////////////////////////////////
// Heap allocator

fn heap_allocator_proc(
    mutaptr    data,
    AllocMode  mode,
    isize      size,
    isize      align,
    mutaptr    old_ptr,
    isize      old_size,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    cx_unused(data);
    switch (mode) {
    case AllocMode_Alloc:
        return heap_aligned_alloc(size, align, null, 0, flags & AllocFlag_Zero);
    case AllocMode_Resize:
        return heap_aligned_resize(old_ptr, old_size, size, align, flags & AllocFlag_Zero);
    case AllocMode_Free:
        heap_aligned_free(old_ptr);
        return {null, none};
    case AllocMode_FreeAll:
        return {old_ptr, Alloc_InvalidMode};
    }
    cx_unreachable;
}

inln fn heap_allocator() noexce -> Allocator
{
    return {heap_allocator_proc, null};
}

////////////////////////////////////////////
// Arena allocator

// TODO:(manu) Extend functionality in relation with threads, virtual memory and context.

struct Arena
{
    byteptr  data{};
    isize    cap{};
    isize    pos{};
};

fn arena_init(Arena& a, isize cap) noexce -> ErrorCode
{
    auto [ptr, err] = mem_alloc(heap_allocator(), cap, DEF_ALIGN, AllocFlag_Default);
    if (err) {
        return err;
    }
    a.data = byteptr(ptr);
    a.cap = cap;
    a.pos = 0;
    return Error_None;
}

fn arena_deinit(Arena& a) noexce -> ErrorCode
{
    auto [_, err] = mem_free(heap_allocator(), a.data);
    a.data = null;
    a.cap = 0;
    a.pos = 0;
    return err;
}

fn arena_allocator_proc(
    mutaptr    data,
    AllocMode  mode,
    isize      size,
    isize      align,
    mutaptr    old_ptr,
    isize      old_size,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    Arena& arena = *cast(Arena*, data);
    if (mode == AllocMode_Free) {
        return {null, Error_None};
    }
    if (mode == AllocMode_FreeAll) {
        arena.pos = 0;
        return {null, Error_None};
    }
    if (mode == AllocMode_Resize && old_ptr != null) {
        auto [ptr, err] = arena_allocator_proc(
            data, AllocMode_Alloc, size, align, null, 0, flags & ~AllocFlag_Zero
        );
        if (err) {
            return {null, err};
        }
        mem_copy(ptr, old_ptr, cx_min2(old_size, size));
        return {ptr, Error_None};
    }
    if (size <= 0 || arena.data == null) {
        return {null, Invalid_Arg};
    }
    if (align <= 0) {
        align = DEF_ALIGN;
    }
    byteptr base = arena.data + arena.pos;
    uptr aligned = align_up(uptr(base), align);
    isize offset = isize(aligned - uptr(arena.data));
    if (offset > arena.cap || size > arena.cap - offset) {
        return {null, Alloc_ExhaustedMem};
    }
    mutaptr ptr = arena.data + offset;
    arena.pos = offset + size;
    if (flags & AllocFlag_Zero) {
        mem_zero(ptr, size);
    }
    return {ptr, Error_None};
}

inln fn arena_allocator(Arena& a) noexce -> Allocator
{
    return {arena_allocator_proc, mutaptr(&a)};
}

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_ALLOCATOR_HH
