#pragma once

#include <string>
#include <vector>

#include "fly/fly.h"

namespace fly {

/**
 * Windows declaration of the SystemImpl interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 2, 2016
 */
class SystemImpl
{
public:
    static void PrintBacktrace();
    static std::string LocalTime(const std::string &);
    static int GetErrorCode();
    static std::string GetErrorString(int);
    static std::vector<int> GetSignals();
};

}
