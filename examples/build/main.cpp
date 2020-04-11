#include <iostream>

#include "some_lib/some_lib.hpp"

int main()
{
    fly::SomeClass sc(15);
    std::cout << sc() << std::endl;

    return 0;
}
