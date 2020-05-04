#pragma once

#include "fly/config/config.hpp"

#include <chrono>
#include <cstdint>

namespace fly {

/**
 * Class to hold configuration values related to the logger.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
class LoggerConfig : public Config
{
public:
    static constexpr const char *identifier = "logger";

    /**
     * Constructor.
     */
    LoggerConfig() noexcept;

    /**
     * @return Max log file size (in bytes) before rotating the log file.
     */
    std::uintmax_t MaxLogFileSize() const noexcept;

    /**
     * @return Max message size (in bytes) per log.
     */
    std::uint32_t MaxMessageSize() const noexcept;

    /**
     * @return Sleep time for logger IO thread.
     */
    std::chrono::milliseconds QueueWaitTime() const noexcept;

protected:
    std::uintmax_t m_defaultMaxLogFileSize;
    std::uint32_t m_defaultMaxMessageSize;
    std::chrono::milliseconds::rep m_defaultQueueWaitTime;
};

} // namespace fly