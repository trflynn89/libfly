#include "c/some_lib/some_lib.h"

#include <assert.h>
#include <stdio.h>

int main()
{
    assert(some_value() == 12389);

    printf("Passed!\n");
    return 0;
}
