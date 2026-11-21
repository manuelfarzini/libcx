#include <stdio.h>

#include "libcx/mem/multi.hh"

int main()
{
#if defined(CX_TEST_MEM_MULTI)
    CX_TEST_RUN(mem_multi_basic);
#endif
}
