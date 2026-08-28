/** @file src/main.cc **/

#include <stdio.h>

#include "libcx/conf/macro.hh"
#include "libcx/math/math.hh"

fn test_dummy() -> void
{
    printf("Hello, this is a dummy test\n");
}

fn test_math() -> i32
{
    i32 x = 1, y = 2, z = 3;
    return cx::max(x, y, z);
}


int main()
{
    test_dummy();
    printf("%d\n", test_math());
}
