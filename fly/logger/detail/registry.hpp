#pragma once

#include <memory>

namespace fly {
class Logger;
} // namespace fly

namespace fly::detail {

/**
 * Singleton class to register and store created loggers. Currently is only used to manage the
 * default logger, but should be extended to manage all loggers.
 *
 * Upon creation, sets the default logger to a synchronous console logger.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 11, 2020
 */
class Registry
{
public:
    /**
     * @return A reference to the singleton registry instance.
     */
    static Registry &instance();

    /**
     * Set the default logger instance for the LOG* macro functions. If the provided logger is null,
     * the default logger is reset to the initial synchronous console logger.
     *
     * @param logger The logger instance.
     */
    void set_default_logger(const std::shared_ptr<Logger> &default_logger);

    /**
     * @return The default logger instance for the LOG* macro functions.
     */
    Logger *get_default_logger() const;

private:
    /**
     * Constructor. Creates the initial default logger.
     */
    Registry();

    std::shared_ptr<Logger> m_initial_default_logger;
    std::shared_ptr<Logger> m_default_logger;
};

} // namespace fly::detail
