/** @file libcx/mem/allocator.hh **/

#ifndef CX_MEM_ALLOCATOR_HH
#define CX_MEM_ALLOCATOR_HH

#include "libcx/conf/macro.hh"
#include "libcx/conf/prelude.hh"
#include "libcx/conf/type.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/pointer.hh"

#include <errno.h>
#include <stdlib.h>

#if CX_SYSTEM_WIN
    #include <malloc.h>
#endif

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

// TODO:(manu) Integrate functionality.
// inln fn mem_free_size(Allocator a, mutaptr ptr, isize size) noexce -> Res<mutaptr, ErrorCode>
// {
//     cx_unused(size);
//     return mem_free(a, ptr);
// }

////////////////////////////////////////////
// Heap allocator

// Selects the portable manually aligned heap strategy instead of the native primitives.
#ifndef CX_MANUAL_HEAP_ALLOC
    #define CX_MANUAL_HEAP_ALLOC 0
#endif

#if CX_MANUAL_HEAP_ALLOC
// Manual heap allocation layout: [base pointer][alignment padding][payload].

inln fn heap_alloc(
    isize      size,
    AllocFlag  flags  =  AllocFlag_Default
) noexce -> mutaptr {
    if (flags & AllocFlag_Zero) {
        return ::calloc(1, usize(size));
    }
    return ::malloc(usize(size));
}

inln fn heap_resize(mutaptr ptr, isize new_size) noexce -> mutaptr
{
    return ::realloc(ptr, usize(new_size));
}

inln fn heap_free(mutaptr ptr) noexce -> void
{
    ::free(ptr);
}

inln fn heap_alloc_aligned(
    isize      size,
    isize      align,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    if (align < DEF_ALIGN) {
        align = DEF_ALIGN;
    }
    // cx_assert(is_power_of_two(align) && "`align` must be a power of 2");

    isize space = PTR_SIZE + (align - 1) + size;
    mutaptr base = heap_alloc(space, flags);
    if (base == null) {
        return {null, Alloc_Exhausted};
    }

    mutaptr ptr = align_up(ptr_add(base, PTR_SIZE), align);
    (cast(void**, ptr))[-1] = base;
    return {ptr, Error_None};
}

inln fn heap_free_aligned(mutaptr ptr) noexce -> void
{
    if (ptr != null) {
        heap_free(cast(void**, ptr)[-1]);
    }
}

// XXX:(manu)
// Alternative manual resize for future benchmarks: reallocate the base block
// with `heap_resize` and realign the payload when `align <= DEF_ALIGN`;
// otherwise fall back to allocate-copy-free. Not used by the initial strategy.

#elif CX_SYSTEM_WIN
// The CRT aligned allocator supports arbitrary power-of-two over-alignment and
// has a matching `_aligned_free`; `HeapAlloc` only guarantees the process heap
// allocation alignment.

inln fn heap_alloc_aligned(
    isize      size,
    isize      align,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    if (align < DEF_ALIGN) {
        align = DEF_ALIGN;
    }
    // cx_assert(is_power_of_two(align) && "`align` must be a power of 2");

    mutaptr ptr = ::_aligned_malloc(usize(size), usize(align));
    if (ptr == null) {
        return {null, Alloc_Exhausted};
    }
    if (flags & AllocFlag_Zero) {
        auto err = mem_zero(ptr, size);
        if (err) {
            ::_aligned_free(ptr);
            return {null, err};
        }
    }
    return {ptr, Error_None};
}

inln fn heap_free_aligned(mutaptr ptr) noexce -> void
{
    if (ptr != null) {
        ::_aligned_free(ptr);
    }
}

#else

// `posix_memalign` is available on Linux, macOS, and the BSDs and, unlike C11
// `aligned_alloc`, does not require `size` to be a multiple of `align`.
inln fn heap_alloc_aligned(
    isize      size,
    isize      align,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    if (align < DEF_ALIGN) {
        align = DEF_ALIGN;
    }
    // cx_assert(is_power_of_two(align) && "`align` must be a power of 2");

    mutaptr ptr = null;
    int err = ::posix_memalign(&ptr, usize(align), usize(size));
    if (err != 0) {
        return {null, err == ENOMEM ? Alloc_Exhausted : Invalid_Arg};
    }
    if (flags & AllocFlag_Zero) {
        auto zero_err = mem_zero(ptr, size);
        if (zero_err) {
            ::free(ptr);
            return {null, zero_err};
        }
    }
    return {ptr, Error_None};
}

inln fn heap_free_aligned(mutaptr ptr) noexce -> void
{
    if (ptr != null) {
        ::free(ptr);
    }
}

#endif

inln fn heap_resize_aligned(
    mutaptr    old_ptr,
    isize      old_size,
    isize      new_size,
    isize      align,
    AllocFlag  flags
) noexce -> Res<mutaptr, ErrorCode> {
    if (old_ptr == null) {
        return heap_alloc_aligned(new_size, align, flags);
    }
    if (new_size <= 0 || old_size < 0) {
        return {old_ptr, Invalid_Arg};
    }
    if (new_size <= old_size) {
        return {old_ptr, Error_None};
    }

    auto [new_ptr, err] = heap_alloc_aligned(new_size, align, flags & ~AllocFlag_Zero);
    if (err) {
        return {old_ptr, err};
    }

    if (old_size > 0) {
        mem_copy(new_ptr, old_ptr, old_size);
    }
    if (flags & AllocFlag_Zero) {
        err = mem_zero(ptr_add(new_ptr, old_size), new_size - old_size);
        if (err) {
            heap_free_aligned(new_ptr);
            return {old_ptr, err};
        }
    }

    heap_free_aligned(old_ptr);
    return {new_ptr, Error_None};
}

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
        return heap_alloc_aligned(size, align, flags);
    case AllocMode_Free:
        heap_free_aligned(old_ptr);
        return {null, Error_None};
    case AllocMode_Resize:
        return heap_resize_aligned(old_ptr, old_size, size, align, flags);
    case AllocMode_FreeAll:
        return {null, Alloc_InvalidMode};
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
    switch (mode) {
    case AllocMode_Alloc: {
        if (size <= 0 || arena.data == null) {
            return {null, Invalid_Arg};
        }
        if (align < DEF_ALIGN) {
            align = DEF_ALIGN;
        }
        // cx_assert(is_power_of_two(align) && "`align` must be a power of 2");

        byteptr base = arena.data + arena.pos;
        uptr aligned = align_up(uptr(base), align);
        isize offset = isize(aligned - uptr(arena.data));
        if (offset > arena.cap || size > arena.cap - offset) {
            return {null, Alloc_Exhausted};
        }

        mutaptr ptr = arena.data + offset;
        if (flags & AllocFlag_Zero) {
            auto err = mem_zero(ptr, size);
            if (err) {
                return {null, err};
            }
        }
        arena.pos = offset + size;
        return {ptr, Error_None};
    }
    case AllocMode_Free:
        return {null, Alloc_InvalidMode};
    case AllocMode_Resize: {
        if (old_ptr == null) {
            return arena_allocator_proc(
                data, AllocMode_Alloc, size, align, null, 0, flags
            );
        }
        if (size <= 0 || old_size < 0) {
            return {old_ptr, Invalid_Arg};
        }
        if (size <= old_size) {
            return {old_ptr, Error_None};
        }

        isize prev_pos = arena.pos;
        auto [ptr, err] = arena_allocator_proc(
            data, AllocMode_Alloc, size, align, null, 0, flags & ~AllocFlag_Zero
        );
        if (err) {
            return {old_ptr, err};
        }

        if (old_size > 0) {
            mem_copy(ptr, old_ptr, old_size);
        }
        if (flags & AllocFlag_Zero) {
            err = mem_zero(ptr_add(ptr, old_size), size - old_size);
            if (err) {
                arena.pos = prev_pos;
                return {old_ptr, err};
            }
        }
        return {ptr, Error_None};
    }
    case AllocMode_FreeAll:
        arena.pos = 0;
        return {null, Error_None};
    }
    cx_unreachable;
}

inln fn arena_allocator(Arena& a) noexce -> Allocator
{
    return {arena_allocator_proc, mutaptr(&a)};
}

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_ALLOCATOR_HH
