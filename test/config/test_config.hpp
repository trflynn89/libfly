#pragma once

#include "fly/config/config.hpp"
#include "fly/types/json/json.hpp"

#include <string>

namespace fly::test {

/**
 * A pseudo config class to provide public access to Config methods. Only meant to be used by unit
 * tests.
 */
class TestConfig : public fly::config::Config
{
public:
    static constexpr char const *identifier = "config";

    template <typename T>
    T get_value(std::string const &name, T def) const
    {
        return fly::config::Config::get_value(name, def);
    }

    void update(fly::Json const &values)
    {
        fly::config::Config::update(values);
    }
};

} // namespace fly::test
