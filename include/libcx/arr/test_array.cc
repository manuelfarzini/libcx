#include <stdio.h>

#include "libcx/arr/dynamic_array.hh"
#include "libcx/arr/inline_array.hh"
#include "libcx/arr/multi_array.hh"
#include "libcx/arr/static_array.hh"

int main()
{
#if defined(CX_TEST_DYNAMIC_ARRAY)
    CX_TEST_RUN(dynamic_array_basic);
#endif
#if defined(CX_TEST_INLINE_ARRAY)
    cx::test_inline_array_compare();
#endif
#if defined(CX_TEST_MULTI_ARRAY)
    CX_TEST_RUN(multi_array_basic);
#endif
}
