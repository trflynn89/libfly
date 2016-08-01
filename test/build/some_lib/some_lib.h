#pragma once

namespace fly {

class SomeClass
{
public:
    SomeClass(int);
    int operator()() const;

private:
    const int m_value;
};

}
