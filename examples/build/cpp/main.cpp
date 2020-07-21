#include "some_lib/some_lib.hpp"

#include <iostream>

int main()
{
    fly::SomeClass sc(15);
    std::cout << sc() << std::endl;

    return 0;
}
