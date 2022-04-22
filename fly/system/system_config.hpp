#pragma once

#include "fly/config/config.hpp"

#include <chrono>

namespace fly::system {

/**
 * Class to hold configuration values related to the system interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class SystemConfig : public fly::config::Config
{
public:
    static constexpr char const *identifier = "system";

    /**
     * @return Delay between system monitor poll intervals.
     */
    virtual std::chrono::milliseconds poll_interval() const;

protected:
    std::chrono::milliseconds::rep m_default_poll_interval {1000};
};

} // namespace fly::system
