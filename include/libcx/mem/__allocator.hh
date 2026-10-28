/** @file mem/allocator.hh **/

#ifndef CX_MEM_ALLOCATOR_HH
#define CX_MEM_ALLOCATOR_HH

#include "libcx/mem/common.hh"
#include "libcx/mem/heap.hh"

namespace cx {
inline namespace mem {

// -----------------------------------------
// Common

enum AllocatorMode : u8 {
    Mode_Alloc,
    Mode_Free,
    Mode_Resize,
    Mode_FreeAll,
    // Mode_AllocTyped,
    // Mode_ResizeTyped,
    // Mode_FreeTyped,
};

typedef auto AllocatorProc(
    mutaptr           alc_data,
    AllocatorMode    mode,
    isize            size,
    isize            align,
    mutaptr           old_ptr,
    isize            old_size,
    b32              zero_mem
) noexce -> Res<mutaptr, ErrorCode>;

#define CX_ALLOCATOR_PROC(name)                   \
    onedef auto name(                             \
        mutaptr           alc_data,                \
        AllocatorMode    mode,                    \
        isize            size,                    \
        isize            align       = DEF_ALIGN, \
        mutaptr           old_ptr     = null,      \
        isize            old_size    = 0,         \
        b32              zero_mem    = true       \
    ) noexce -> Res<mutaptr, ErrorCode>

struct Allocator {
    AllocatorProc*    call;
    mutaptr            data;
};

fn cx_mem_alloc(
    Allocator alctor, isize size, isize align = DEF_ALIGN, b32 zero_mem = true
) noexce -> Res<mutaptr, ErrorCode> {
    if (size == 0) {
        return {null, Invalid_Arg};
    }
    return alctor.call(alctor.data, Mode_Alloc, size, align, null, 0, zero_mem);
}

inln fn cx_mem_free(Allocator alctor, mutaptr ptr) -> Res<mutaptr, ErrorCode>
{
    return alctor.call(alctor.data, Mode_Free, 0, 0, ptr, 0, true);
}

inln fn cx_mem_resize(
    Allocator    alctor, 
    mutaptr       ptr,
    isize        old_size,
    isize        new_size,
    isize        align        = DEF_ALIGN 
) noexce -> Res<mutaptr, ErrorCode> {
    if (new_size == 0) {
        if (ptr != null) {
            return alctor.call(alctor.data, Mode_Free, 0, 0, ptr, old_size, true);
        }
        return {null, Invalid_Arg};
    } else if (ptr == null) {
        return alctor.call(alctor.data, Mode_Alloc, new_size, align, null, 0, true);
    } else if (old_size == new_size && uptr(ptr) % uptr(align) == 0) {
        return alctor.call(alctor.data, Mode_Alloc, new_size, align, null, 0, false);
    }
    return alctor.call(alctor.data, Mode_Resize, new_size, align, ptr, old_size, true);
}

inln fn cx_mem_free_all(Allocator alctor) noexce -> Res<mutaptr, ErrorCode>
{
    return alctor.call(alctor.data, Mode_FreeAll, 0, 0, null, 0, true);
}

/**
    @par
    - `size`: the size of the memory to be freed [bytes]
**/
fn cx_mem_free_size(
    Allocator alctor, mutaptr ptr, isize size
) noexce -> Res<mutaptr, ErrorCode> {
    return alctor.call(alctor.data, Mode_Free, 0, 0, ptr, size, true);
}

//------------------------------------------
// Heap

CX_ALLOCATOR_PROC(heap_allocator_proc)
{
    cx_unused(alc_data);

    switch (mode) {
        case Mode_Alloc:
            return heap_aligned_alloc(size, align, old_ptr, old_size, zero_mem);

        case Mode_Free: 
            heap_aligned_free(old_ptr);
            return {null, none};

        case Mode_Resize:
            return heap_aligned_resize(old_ptr, old_size, size, align, zero_mem);

        case Mode_FreeAll:
            return {null, Alloc_Invalid_Mode};
 
        // case Mode_AllocTyped:
        //     return heap_aligned_alloc(size, align, old_ptr, old_size, false);

        // case Mode_FreeTyped:
        //     heap_aligned_free(old_ptr);
        //     return {null, none};

        // case Mode_ResizeTyped:
        //     return heap_aligned_resize(old_ptr, old_size, size, align, false);

        default:
            // TODO:(manu) Should I use something like cx_panic()?
            cx_unreachable();
    }
}

inln priv cons fn heap_allocator() -> Allocator
{
    return Allocator{heap_allocator_proc, null};
}

/*                                         *
* Make                                     *
*                                         */

/**
    TODO: write the correct concept
**/
template<typename T>
nodisc fn make_array(
    isize num, Allocator alctor = heap_allocator(), isize align = size_of(T)
) noexce -> Res<T*, ErrorCode> {
    // if constexpr (is_zeroable<T>) {
    //     auto [ptr, err] = cx_alloc(alctor, num * size_of(T), align);
    //     if (err) {
    //         return {null, err};
    //     }
    //     return {cast(T*, ptr), err};
    //} else {
        auto [ptr, err] = cx_mem_alloc(alctor, num * size_of(T), align, false);
        if (err) {
            return {null, err};
        }
        init_type<T>(ptr, num);
        return {cast(T*, ptr), none};
    // }
}

template<typename T>
nodisc fn delete_array(
    T* ptr, isize num, Allocator alctor = heap_allocator()
) noexce -> Res<mutaptr, ErrorCode> {
    auto [_, err] = deinit_type<T>(ptr, num);
    if (err) {
        return {_, err};
    }
    return cx_mem_free_size(alctor, ptr, num);
}

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_ALLOCATOR_HH


////////////////////////////////////////////////////////////////
//
//
//
// Unused
//
//

// NOTE:(manu) - old
// We have also the typed version of the allocator modes in
// to value init non primitive types within the `make_type` fn.
// This can be further extended with the different init_type procs.
// Is this is a good idea? Should the non strictly zero init be done
// explicitly caller side? Is more code actually useful?
// Consider that typed versions are useful with containers (arrays, maps, ...)


// /**
//   Allocates memory and places `num` contiguous `T` objects using `args`.
//
//   @Arguments
//   - `num`: the number of objects
//
//   @Returns
//   - `null` if the allocation fails or `num == 0`
//   - `ptr` pointer otherwise
// **/
// template<typename T> nodisc onedef fn heap_make_type(isize num) noexce -> Res<T*, ErrorCode> {
//     auto [ptr, err] = heap_aligned_alloc_type<T>(num);
//     if (err) {
//         return {null, err};
//     }
//     return init_ls<T>(ptr, num);
// }

// /**
//   Deinitializes `num` contiguous `T` objects at `ptr` and frees the memory.
//
//   @Parameters
//   - `ptr`: the pointer to the first element
//   - `num`: the number of elements to deinitialize
//
//   @Returns
//   - `{null, err}` if `ptr` is `null` or `num` is `0`;
//   - `{ptr, none}` otherwise
//
//   @Requires
//   - `ptr` is properly aligned
//   - `ptr` has `num` elements
// **/
// template<typename T>
// onedef fn heap_remove_type(T* ptr, isize num) noexce -> Res<Empty, ErrorCode> {
//     auto [_, err] = deinit_type<T>(ptr, num);
//     if (err) {
//         return {_, uti::take(err)};
//     }
//     return heap_aligned_free(ptr);
// }
