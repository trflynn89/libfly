#pragma once

#include "fly/config/config.hpp"

#include <chrono>

namespace fly {

/**
 * Class to hold configuration values related to the system interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
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
    virtual std::chrono::milliseconds poll_interval() const;

protected:
    std::chrono::milliseconds::rep m_default_poll_interval;
};

} // namespace fly
