#pragma once

#include <string>
#include <vector>

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
    static void PrintBacktrace() noexcept;
    static std::string LocalTime(const std::string &) noexcept;
    static int GetErrorCode() noexcept;
    static std::string GetErrorString(int) noexcept;
    static std::vector<int> GetSignals() noexcept;
};

} // namespace fly
