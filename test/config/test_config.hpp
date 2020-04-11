#pragma once

#include "fly/config/config.hpp"
#include "fly/types/json/json.hpp"

#include <string>

/**
 * A pseudo config class to provide public access to Config methods. Only meant
 * to be used by unit tests.
 */
class TestConfig : public fly::Config
{
public:
    static constexpr const char *identifier = "config";

    template <typename T>
    T GetValue(const std::string &name, T def) const noexcept
    {
        return fly::Config::GetValue(name, def);
    }

    void Update(const fly::Json &values) noexcept
    {
        fly::Config::Update(values);
    }
};
