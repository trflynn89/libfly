#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace fly {
class Logger;
} // namespace fly

namespace fly::detail {

/**
 * Singleton class to register and store created loggers. Upon creation, sets the default logger to
 * a synchronous console logger.
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
    void set_default_logger(const std::shared_ptr<fly::Logger> &default_logger);

    /**
     * @return The default logger instance for the LOG* macro functions.
     */
    fly::Logger *get_default_logger() const;

    /**
     * Register a logger instance. If the given logger's name is already registered, this
     * registration will fail.
     *
     * @param logger The logger instance to register.
     *
     * @return True if the logger could be registered.
     */
    bool register_logger(const std::shared_ptr<fly::Logger> &logger);

    /**
     * Remove a logger instance from the registry.
     *
     * @param name Name of the logger to unregister.
     */
    void unregister_logger(const std::string &name);

    /**
     * Retrieve a logger from the registry. If the logger is not found, or if the logger instance
     * has been deleted since it was registered, returns null.
     *
     * @param name Name of the logger to retrieve.
     *
     * @return The logger instance, or null.
     */
    std::shared_ptr<fly::Logger> get_logger(const std::string &name);

private:
    /**
     * Constructor. Creates the initial default logger.
     */
    Registry();

    /**
     * Destructor. Clear the registry.
     */
    ~Registry();

    std::shared_ptr<fly::Logger> m_initial_default_logger;
    std::shared_ptr<fly::Logger> m_default_logger;

    std::mutex m_registry_mutex;
    std::map<std::string, std::weak_ptr<fly::Logger>> m_registry;
};

} // namespace fly::detail
