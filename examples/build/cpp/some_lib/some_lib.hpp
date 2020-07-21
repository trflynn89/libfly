#pragma once

namespace fly {

class SomeClass
{
public:
    SomeClass(int value);
    int operator()() const;

private:
    const int m_value;
};

} // namespace fly
