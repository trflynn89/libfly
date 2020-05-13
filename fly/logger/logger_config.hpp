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
    std::uintmax_t max_log_file_size() const noexcept;

    /**
     * @return Max message size (in bytes) per log.
     */
    std::uint32_t max_message_size() const noexcept;

    /**
     * @return Sleep time for logger IO thread.
     */
    std::chrono::milliseconds queue_wait_time() const noexcept;

protected:
    std::uintmax_t m_default_max_log_file_size;
    std::uint32_t m_default_max_message_size;
    std::chrono::milliseconds::rep m_default_queue_wait_time;
};

} // namespace fly
