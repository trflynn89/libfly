#include "some_lib.h"

namespace fly {

//==============================================================================
SomeClass::SomeClass(int value) : m_value(value)
{
}

//==============================================================================
int SomeClass::operator()() const
{
    return m_value;
}

}
