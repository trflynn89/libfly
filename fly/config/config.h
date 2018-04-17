#pragma once

#include <map>
#include <shared_mutex>

#include "fly/fly.h"
#include "fly/parser/json.h"
#include "fly/parser/parser.h"

namespace fly {

FLY_CLASS_PTRS(Config);

/**
 * Class to hold a set of related configuration values.
 *
 * Classes may derive from this class and define helper getter functions for
 * each of its config values. Any derived class must define a static GetName()
 * method.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class Config
{
public:
    /**
     * Constructor.
     */
    Config();

    /**
     * Destructor.
     */
    virtual ~Config();

    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * Get a value converted to a basic type, e.g. int or bool. If the value
     * could not be found, or could not be converted to the given type, returns
     * the provided default value.
     *
     * @tparam T The basic return type of the value.
     *
     * @param string The name of the value.
     * @param T Default value to use if the value could not be found or converted.
     *
     * @return The converted value or the default value.
     */
    template <typename T>
    T GetValue(const std::string &, T) const;

    /**
     * Update this configuration with a new set of parsed values.
     */
    void Update(const Json &);

private:
    mutable std::shared_timed_mutex m_valuesMutex;
    Json m_values;
};

//==============================================================================
template <typename T>
T Config::GetValue(const std::string &name, T def) const
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

}
