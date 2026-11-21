/** @file libcx/mem/pointer.hh **/

#ifndef CX_MEM_POINTER_HH
#define CX_MEM_POINTER_HH

#include "libcx/config.hh"
#include "libcx/concept/type.hh"

namespace cx {
inline namespace mem {

// TODO:(manu) Implement without relying on standrd library
//  /**
//      Prints the full address of `ptr` in hex.
//
//      @arg
//      - `ptr`: the pointer to print
//  **/
//  onedef fn print_full_ptr(mutaptr ptr) noexce -> void
//  {
//      printf("%0*" PRIXPTR "\n", int(size_of(uptr) * 2), uptr(ptr));
//  }

/**
    Aligns `ptr` upward to the `aln` boundary.
    @arg
    - `ptr`: the pointer
    - `aln`: the alignment.
    @ret
    - The aligned pointer.
    @req
    - `aln` is a power of two.
**/
inln cons fn align_up(mutaptr ptr, isize aln) noexce -> mutaptr
{
    // assert(uti::is_power_of_two(aln));
    return mutaptr((uptr(ptr) + (aln - 1)) & ~(aln - 1));
}

/**
    Aligns `ptr` upward to the `aln` boundary.
    @arg
    - `ptr`: the pointer.
    - `aln`: the alignment.
    @ret
    - The aligned pointer.
    @req
    - `aln` is a power of two.
**/
inln cons fn align_up(uptr ptr, isize aln) noexce -> uptr
{
    // cx_assert(is_power_of_two(aln));  // XXX:(manu)
    return (ptr + (aln - 1)) & ~(uptr(aln - 1));
}

/**
    Aligns `off` upward to the `aln` boundary.
    @arg
    - `off`: the offset or size in bytes.
    - `aln`: the alignment.
    @ret
    - The aligned offset or size.
    @req
    - `aln` is a power of two.
**/
inln cons fn align_up(isize off, isize aln) noexce -> isize
{
    // assert(uti::is_power_of_two(aln));
    return (off + (aln - 1)) & ~(aln - 1);
}

/**
    Aligns `ptr` upward to the alignment of `Tp`.
    @arg
    - `Tp`: the type to align to.
    - `ptr`: the pointer.
    @ret
    - The aligned pointer.
**/
template<typename Tp>
inln cons fn align_up(mutaptr ptr) noexce -> mutaptr
{
    return align_up(ptr, align_of(Tp));
}

/**
    Aligns `ptr` upward to the alignment of `Tp`.
    @arg
    - `Tp`: the type to align to.
    - `ptr`: the pointer.
    @ret
    - The aligned pointer.
**/
template<typename Tp>
inln cons fn align_up(uptr ptr) noexce -> uptr
{
    return align_up(ptr, align_of(Tp));
}

/**
    Aligns `off` upward to the alignment of `Tp`.
    @arg
    - `Tp`: the type to align to.
    - `off`: the initial offset.
    @ret
    - The aligned offset.
**/
template<typename Tp>
inln cons fn align_up(isize off) noexce -> isize
{
    return align_up(off, align_of(Tp));
}

/**
    Adds `off` to `ptr` and returns the result.
    @arg
    - `ptr`: the pointer.
    - `off`: the offset.
    @ret
    - The result pointer.
**/
inln cons fn ptr_add(auto* ptr, isize off) noexce -> auto*
{
    return declt(ptr)(byteptr(ptr) + off);
}

/**
    Subtracts `off` from `ptr` and returns the result.
    @arg
    - `ptr`: the pointer.
    - `off`: the offset.
    @ret
    - the result pointer.
**/
inln cons fn ptr_sub(auto* ptr, isize off) noexce -> auto*
{
    return declt(ptr)(byteptr(ptr) - off);
}

/**
    Computes the difference between `end` and `beg`.
    @arg
    - `beg`: the beginning pointer.
    - `end`: the end pointer.
    @ret
    - The difference in bytes.
**/
inln cons fn ptr_diff(auto* beg, auto* end) noexce -> isize
{
    return isize(byteptr(end) - byteptr(beg));
}

/**
    Tags the given pointer with the given `tag`.
    @arg
    - `ptr`: the pointer.
    - `tag`: the tag.
    @ret
    - The tagged pointer.
**/
template<SomeRawPointer Ptr>
inln cons fn ptr_tag(mutaptr ptr, uptr tag) noexce -> Ptr
{
    return Ptr(uptr(ptr) | tag);
}

/**
    Removes the tag from the given pointer.
    @arg
    - `ptr`: the pointer.
    - `tag`: the tag.
    @ret
    - The untagged pointer.
**/
template<SomeRawPointer Ptr>
inln cons fn ptr_untag(mutaptr ptr, uptr msk) noexce -> Ptr
{
    return Ptr(uptr(ptr) & ~msk);
}

}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_POINTER_HH
