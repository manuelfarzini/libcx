#include <stdio.h>

#include "libcx/uti/tuple.hh"

int main()
{
#if defined(CX_TEST_TUPLE)
    CX_TEST_RUN(tuple_basic);
#endif
}
