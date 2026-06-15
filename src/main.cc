/** @file src/main.cc **/

#include <stdio.h>

#include "libcx/conf/macro.hh"

fn test_dummy() -> void
{
    printf("Hello, this is a dummy test\n");
}

int main()
{
    test_dummy();
}
