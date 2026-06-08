/** @file libcx/mem/heap.hh **/

#ifndef CX_MEM_HEAP_HH
#define CX_MEM_HEAP_HH

#include "libcx/conf.hh"
#include "libcx/__utils/ownership.hh"
#include "libcx/mem/common.hh"
#include "libcx/mem/pointer.hh"

namespace cx {
inline namespace mem {

cons fn heap_free(mutaptr ptr) noexce -> void
{
    return ::free(ptr);
}

cons fn heap_alloc(isize size, b32 zero_mem = true) noexce -> mutaptr
{
    if (size <= 0) {
        return null;
    }

    if (zero_mem) {
        return ::calloc(1, size);
    } else {
        return ::malloc(size);
    }
}

cons fn heap_resize(mutaptr ptr, isize new_size) noexce -> mutaptr
{
    return ::realloc(ptr, new_size);
}

cons fn heap_aligned_free(mutaptr ptr) noexce -> void
{
    if (ptr != null) {
        ::free(cast(void** ,ptr)[-1]);
    }
}

nodisc cons fn heap_aligned_alloc(
    isize     size,
    isize     align       = DEF_ALIGN,
    mutaptr    old_ptr     = null,
    isize     old_size    = 0,
    b32       zero_mem    = true
) noexce -> Res<mutaptr, ErrorCode> {
    //  NOTE(manu)
    //  should I impose bytes % align == 0 like std::aligned_alloc?

    if (size <= 0) {
        return {null, Invalid_Arg};
    }
    while (align < DEF_ALIGN) {
        align = align << 1;
    }
    // assert(uti::is_power_of_two(align) && "`align` must be a power of 2");

    isize space = align - 1 + size + PTR_SIZE;
    b32 force_copy = (old_ptr != null) && (align > PTR_ALIGN);

    mutaptr alloced_mem = null;
    if (old_ptr != null && !force_copy) {
        mutaptr origin_ptr = cast(void** ,old_ptr)[-1];
        alloced_mem = heap_resize(origin_ptr, space);
    } else {
        alloced_mem = heap_alloc(space, zero_mem);
    }

    if (alloced_mem == null) {
        heap_aligned_free(old_ptr);
        heap_aligned_free(alloced_mem);
        return {null, Alloc_Exhausted};
    }

    mutaptr aligned_mem = ptr_add(alloced_mem, PTR_SIZE);
    uptr aligned_ptr = (uptr(aligned_mem) + align - 1) & ~(align - 1);
    aligned_mem = mutaptr(aligned_ptr);
    (cast(void** ,aligned_mem))[-1] = alloced_mem;

    if (force_copy) {
        mem_move(aligned_mem, old_ptr, cx_min2(size, old_size));
        heap_aligned_free(old_ptr);
    }

    return {aligned_mem, none};
}

cons fn heap_aligned_resize(
    mutaptr    ptr,
    isize     size,
    isize     new_size,
    isize     new_align,
    b32       zero_mem    = true
) noexce -> Res<mutaptr, ErrorCode> {
    if (ptr == null) {
        return heap_aligned_alloc(new_size, new_align, null, size, zero_mem);
    }
    auto [new_ptr, err] = heap_aligned_alloc(new_size, new_align, ptr, size, zero_mem);
    if (err) {
        return {ptr, err};
    }

    if (zero_mem && new_size > size) {
        //  actually using std malloc and realloc, not os specific primitives, so
        //  the new_region should be zeroed in windows too

        mutaptr new_region = ptr_add(new_ptr, size);

        //  if not dealing with vm paging plain mem_zero is faster than conditional
        mem_zero(new_region, new_size - size);
        //  condit_mem_zero(new_region, new_size - old_size);
    }

    return {new_ptr, none};
}

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_HEAP_HH


////////////////////////////////////////////////////////////////
//
//
//
// Unused
//
//

// template<DefInitble T>
// nodisc cons fn heap_aligned_alloc_type(
//     isize     num        = 1,
//     isize     align      = align_of(T),
//     mutaptr    old_ptr    = null,
//     isize     old_num    = 0
// ) noexce -> Res<T*, AllocatorError> {
//     auto [new_ptr, err] = heap_aligned_alloc(
//         num * size_of(T), 
//         align, old_ptr, 
//         old_num * size_of(T), 
//         false);
//     if (err) {
//         return {null, err};
//     }
//     init_type<T>(new_ptr, num);  // TODO: directly use va version?
//     return {cast(T*,new_ptr), none};
// }
//
//
// /**
//   Reallocates the memory owned by `ptr` from `num` elements of type `T` to
//   `new_num` elements of type `T`
//
//   @Parameters
//   - `ptr`: the pointer to reallocate
//   - `num`: the number of elements to reallocate
//   - `new_num`: the new number of elements
//
//   @Returns
//   - `{null, err}` if the allocation fails
//   - `{ptr, none}` otherwise
//
//   @Requires
//   - `ptr` is properly aligned and has `num` elements
//
// **/
// template<typename T>
// nodisc fn heap_aligned_resize_type(T* ptr, isize num, isize new_num) noexce
//     -> Res<T*, AllocatorError>
// {
//   auto [new_ptr, err] = heap_aligned_alloc_type<T>(new_num, align_of(T), ptr, num);
//   if (err) {
//       return {ptr, err};
//   }
//
//   init_type<T>(cast(T*, new_ptr) + num, new_num - num);  // XXX: should add private unchecked version?
//
//   return {cast(T*, new_ptr), null};
// }
