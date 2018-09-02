#pragma once

#include <sstream>
#include <streambuf>

namespace fly {

/**
 * RAII helper class to redirect std::cout to a stringstream for inspection.
 * Only meant to be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class CaptureCout
{
public:
    /**
     * Constructor. Redirect std::cout to a stringstream and store the
     * original streambuf target.
     */
    CaptureCout();

    /**
     * Destructor. Restore std::cout to the original streambuf target.
     */
    ~CaptureCout();

    /**
     * @return string The contents of std::cout.
     */
    std::string operator() () const;

private:
    std::stringstream m_target;
    std::streambuf *m_original;
};

}
