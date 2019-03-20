#pragma once

#include "fly/config/config.h"

#include <chrono>

namespace fly {

/**
 * Class to hold configuration values related to the system interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class SystemConfig : public Config
{
public:
    static constexpr const char *identifier = "system";

    /**
     * Constructor.
     */
    SystemConfig() noexcept;

    /**
     * @return Delay between system monitor poll intervals.
     */
    virtual std::chrono::milliseconds PollInterval() const noexcept;

protected:
    std::chrono::milliseconds::rep m_defaultPollInterval;
};

} // namespace fly
