/** @file libcx/mem/pointer.hh **/

#ifndef CX_MEM_POINTER_HH
#define CX_MEM_POINTER_HH

#include "libcx/conf.hh"

namespace cx {
inline namespace mem {

// /**
//     Prints the full address of `ptr` in hex.
//   
//     @para
//     - `ptr`: the pointer to print
// **/
// onedef fn print_full_ptr(mutaptr ptr) noexce -> void
// {
//     printf("%0*" PRIXPTR "\n", int(size_of(uptr) * 2), uptr(ptr));
// }

/**
    Aligns `ptr` upward to the `aln` boundary.
    @para
    - `ptr`: the pointer
    - `aln`: the alignment.
    @ret 
    - The aligned pointer.
    @pre
    - `aln` is a power of two.
**/  
inln cons fn align_up(mutaptr ptr, isize aln) noexce -> mutaptr
{
    // assert(uti::is_power_of_two(aln));
    return mutaptr((uptr(ptr) + (aln - 1)) & ~(aln - 1));
}

/**
    Aligns `ptr` upward to the `aln` boundary.
    @para
    - `ptr`: the pointer.
    - `aln`: the alignment.
    @ret 
    - The aligned pointer.
    @pre
    - `aln` is a power of two.
**/  
inln cons fn align_up(uptr ptr, isize aln) noexce -> uptr
{
    // assert(is_power_of_two(aln)); // XXX: 
    return (ptr + (aln - 1)) & ~(uptr(aln - 1));
}

/**
    Aligns `off` upward to the `aln` boundary.
    @para
    - `off`: the offset or size in bytes.
    - `aln`: the alignment.
    @ret
    - The aligned offset or size.
    @pre
    - `aln` is a power of two.
**/
inln cons fn align_up(isize off, isize aln) noexce -> isize
{
    // assert(uti::is_power_of_two(aln));
    return (off + (aln - 1)) & ~(aln - 1);
}

/**
    Aligns `ptr` upward to the alignment of `Tp`.
    @para
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
  @para
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
    @para
    - `Tp`: the type to align to.
    - `off`: the offset or size in bytes.
    @ret
    - The aligned offset or size.
**/
template<typename Tp>
inln cons fn align_up(isize off) noexce -> isize
{
    return align_up(off, align_of(Tp));
}

/**
  Adds `off` to `ptr` and returns the result.
  @para
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
    @para
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
    @para
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
    @para
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
    @para
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
