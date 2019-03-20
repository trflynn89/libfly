#pragma once

#include "fly/types/json.h"

#include <shared_mutex>
#include <string>

namespace fly {

/**
 * Class to hold a set of related configuration values.
 *
 * Configuration classes must derive from this class and define helper getter
 * functions for each of its config values. Any derived class must define a
 * constexpr C-string named "identifier" to uniquely identify that class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class Config
{
protected:
    friend class ConfigManager;

    /**
     * Constructor.
     */
    Config() noexcept = default;

    /**
     * Destructor.
     */
    virtual ~Config() = default;

    /**
     * Get a value converted to a basic type, e.g. int or bool. If the value
     * could not be found, or could not be converted to the given type, returns
     * the provided default value.
     *
     * @tparam T The basic return type of the value.
     *
     * @param string The name of the value.
     * @param T Default value to use if needed.
     *
     * @return The converted value or the default value.
     */
    template <typename T>
    T GetValue(const std::string &, T) const noexcept;

    /**
     * Update this configuration with a new set of parsed values.
     */
    void Update(const Json &) noexcept;

private:
    mutable std::shared_timed_mutex m_valuesMutex;
    Json m_values;
};

//==============================================================================
template <typename T>
T Config::GetValue(const std::string &name, T def) const noexcept
{
    std::shared_lock<std::shared_timed_mutex> lock(m_valuesMutex);

    try
    {
        return T(m_values[name]);
    }
    catch (const JsonException &)
    {
    }

    return def;
}

} // namespace fly
