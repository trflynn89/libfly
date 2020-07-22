#include "cpp/some_lib/some_lib.hpp"

namespace fly {

//==================================================================================================
SomeClass::SomeClass(int value) : m_value(value)
{
}

//==================================================================================================
int SomeClass::operator()() const
{
    return m_value;
}

} // namespace fly
