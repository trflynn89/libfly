#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/format.hpp"

namespace fly {

//==================================================================================================
JsonException::JsonException(std::string &&message) noexcept :
    m_message(fly::string::format("JsonException: {}", message))
{
}

//==================================================================================================
JsonException::JsonException(Json const &json, std::string &&message) noexcept :
    m_message(fly::string::format("JsonException: {}: ({})", message, json))
{
}

//==================================================================================================
JsonException::JsonException(char const *class_name, std::string &&message) noexcept :
    m_message(fly::string::format("{}: {}", class_name, message))
{
}

//==================================================================================================
char const *JsonException::what() const noexcept
{
    return m_message.c_str();
}

//==================================================================================================
JsonIteratorException::JsonIteratorException(Json const &json, std::string &&message) noexcept :
    JsonException("JsonIteratorException", fly::string::format("{}: ({})", message, json))
{
}

//==================================================================================================
BadJsonComparisonException::BadJsonComparisonException(
    Json const &json1,
    Json const &json2) noexcept :
    JsonException(
        "BadJsonComparisonException",
        fly::string::format(
            "Cannot compare iterators of different JSON instances: ({}) ({})",
            json1,
            json2))
{
}

//==================================================================================================
NullJsonException::NullJsonException(Json const &json) noexcept :
    JsonException(
        "NullJsonException",
        fly::string::format("Cannot dereference an empty or past-the-end iterator: ({})", json))
{
}

//==================================================================================================
NullJsonException::NullJsonException() noexcept :
    JsonException(
        "NullJsonException",
        fly::string::format("Cannot dereference an empty or past-the-end iterator"))
{
}

//==================================================================================================
OutOfRangeJsonException::OutOfRangeJsonException(Json const &json, std::ptrdiff_t offset) noexcept :
    JsonException(
        "OutOfRangeJsonException",
        fly::string::format("Offset {} is out-of-range: ({})", offset, json)),
    m_offset(offset)
{
}

//==================================================================================================
std::ptrdiff_t OutOfRangeJsonException::offset() const
{
    return m_offset;
}

} // namespace fly
