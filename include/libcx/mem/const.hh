#include "libcx/conf/prelude.hh"
#include "libcx/arr/inline_array.hh"


// template<isize S, auto tag, isize A = align_of(u8)>
// align_as(A) priv u8 static_buff[S]{};
//
// cons byteptr global_buff = static_buff<STATIC_ALLOC_LIMIT, clos_tag>;
// cons byteptr another_buff = static_buff<STATIC_ALLOC_LIMIT, clos_tag>;

namespace cx {
inline namespace mem {

cons usize STATIC_ALLOC_LIMIT = 1024 * 1024 * 1024;

#define clos_tag []{}

template<isize Cap>
struct ConsAllocator {
    CX_DEFINE_MEMBER_TYPES(u8, isize);
    onedef glob cons isize cap{Cap};
    InlineArray<byteptr, cap> buf{};
};

CX_CONCEPT_GEN_TEMPL(
    ConsAllocator, is_static_allocator, SomeStaticAllocator, isize Cap, Cap
);

comp fn get_cap(SomeStaticAllocator auto& alc) -> isize { return alc.buf.cap; }
comp fn get_len(SomeStaticAllocator auto& alc) -> isize { return alc.buf.len; }


template<isize Cap>
comp fn cons_allocator() -> ConsAllocator<Cap> { return {}; }

}
}

#define CX_TEST_CONST_ALLOCATOR 1
#if CX_TEST_CONST_ALLOCATOR

using namespace cx;

comp fn test_const_alloc_base() -> void
{
    cons auto buf = cons_allocator<STATIC_ALLOC_LIMIT>();
    cons auto cap = get_cap(buf);
    cons auto len = get_len(buf);
    static_assert(cap == STATIC_ALLOC_LIMIT);
    static_assert(len == 0);
}

comp fn test_launch_const_alloc_base() -> void
{
    test_const_alloc_base();
}

#endif

