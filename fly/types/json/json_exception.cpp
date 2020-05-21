#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"

namespace fly {

//==============================================================================
JsonException::JsonException(const std::string &message) noexcept :
    m_message(String::format("JsonException: %s", message))
{
}

//==============================================================================
JsonException::JsonException(
    Json::const_reference json,
    const std::string &message) noexcept :
    m_message(String::format("JsonException: %s (%s)", message, json))
{
}

//==============================================================================
const char *JsonException::what() const noexcept
{
    return m_message.c_str();
}

} // namespace fly
