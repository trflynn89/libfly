#include "cpp/some_lib/some_lib.hpp"

#include <cassert>
#include <iostream>

int main()
{
    fly::SomeClass sc(15);
    assert(sc() == 15);

    std::cout << "Passed!\n";
    return 0;
}
