#include "test/util/capture_cout.h"

#include <iostream>

namespace fly {

//==============================================================================
CaptureCout::CaptureCout() : m_original(std::cout.rdbuf(m_target.rdbuf()))
{
}

//==============================================================================
CaptureCout::~CaptureCout()
{
    std::cout.rdbuf(m_original);
}

//==============================================================================
std::string CaptureCout::operator() () const
{
    return m_target.str();
}

}
