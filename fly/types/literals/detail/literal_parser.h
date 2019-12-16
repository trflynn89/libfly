#pragma once

#include "fly/traits/traits.h"

#include <limits>
#include <type_traits>

namespace fly::detail {

template <typename T, char Digit>
constexpr T parse_literal()
{
    if constexpr ((Digit >= 'A') && (Digit <= 'F'))
    {
        return static_cast<T>(Digit) - static_cast<T>('A') + 10;
    }
    else if constexpr ((Digit >= 'a') && (Digit <= 'f'))
    {
        return static_cast<T>(Digit) - static_cast<T>('a') + 10;
    }
    else
    {
        static_assert((Digit >= '0') && (Digit <= '9'), "Invalid literal");
        return static_cast<T>(Digit) - static_cast<T>('0');
    }
}

template <typename T, T Base>
constexpr T aggregate(T aggregated)
{
    return aggregated;
}

template <typename T, T Base, char Digit, char... Literals>
constexpr T aggregate(T aggregated)
{
    constexpr T digit = parse_literal<T, Digit>();
    static_assert(digit < Base, "Invalid literal for base");

    return aggregate<T, Base, Literals...>(aggregated * Base + digit);
}

template <typename T, char... Literals>
struct parser
{
    static constexpr T parse()
    {
        return aggregate<T, 10, Literals...>(0);
    }
};

template <typename T, char... Literals>
struct parser<T, '0', 'b', Literals...>
{
    static constexpr T parse()
    {
        return aggregate<T, 2, Literals...>(0);
    }
};

template <typename T, char... Literals>
struct parser<T, '0', 'B', Literals...>
{
    static constexpr T parse()
    {
        return aggregate<T, 2, Literals...>(0);
    }
};

template <typename T, char... Literals>
struct parser<T, '0', Literals...>
{
    static constexpr T parse()
    {
        return aggregate<T, 8, Literals...>(0);
    }
};

template <typename T, char... Literals>
struct parser<T, '0', 'x', Literals...>
{
    static constexpr T parse()
    {
        return aggregate<T, 16, Literals...>(0);
    }
};

template <typename T, char... Literals>
struct parser<T, '0', 'X', Literals...>
{
    static constexpr T parse()
    {
        return aggregate<T, 16, Literals...>(0);
    }
};

template <typename From, typename To, From value>
constexpr To validate_and_convert()
{
    static_assert(value >= std::numeric_limits<To>::min(), "Literal overflow");
    static_assert(value <= std::numeric_limits<To>::max(), "Literal overflow");

    return static_cast<To>(value);
}

template <typename T, char... Literals>
constexpr T literal()
{
    if constexpr (std::is_signed_v<T>)
    {
        constexpr auto value = parser<long long, Literals...>::parse();
        return validate_and_convert<decltype(value), T, value>();
    }
    else
    {
        constexpr auto value = parser<unsigned long long, Literals...>::parse();
        return validate_and_convert<decltype(value), T, value>();
    }
}

} // namespace fly::detail
