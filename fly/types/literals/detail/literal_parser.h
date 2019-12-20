#pragma once

#include <limits>
#include <type_traits>

namespace fly::detail {

/**
 * Structures to recursively aggregate the value of an integer literal as it is
 * being parsed.
 *
 * @tparam T The desired integer literal type.
 * @tparam Base The base of the integer literal being parsed.
 * @tparam Literals Variadic list of characters to parse.
 */
template <typename T, T Base, char... Literals>
struct aggregator;

// Specialization to convert the current character being parsed to an integer
// and recurse on the remainder of the characters.
template <typename T, T Base, char Digit, char... Literals>
struct aggregator<T, Base, Digit, Literals...>
{
    constexpr static T aggregate(T aggregated)
    {
        constexpr T digit = parse_and_validate_literal();
        static_assert(digit < Base, "Invalid literal for base");

        return aggregator<T, Base, Literals...>::aggregate(
            aggregated * Base + digit);
    }

private:
    /**
     * Convert the current character being parsed to an integer literal.
     * Validate that the character is valid.
     *
     * @return T The result of conversion.
     */
    constexpr static T parse_and_validate_literal()
    {
        if constexpr ((Digit >= 'A') && (Digit <= 'F'))
        {
            return static_cast<T>(Digit) - static_cast<T>('A') + 0xA;
        }
        else if constexpr ((Digit >= 'a') && (Digit <= 'f'))
        {
            return static_cast<T>(Digit) - static_cast<T>('a') + 0xa;
        }
        else
        {
            static_assert((Digit >= '0') && (Digit <= '9'), "Invalid literal");
            return static_cast<T>(Digit) - static_cast<T>('0');
        }
    }
};

// Specialization to simply ignore a separator character and recurse on the
// remainder of the characters.
template <typename T, T Base, char... Literals>
struct aggregator<T, Base, '\'', Literals...>
{
    constexpr static T aggregate(T aggregated)
    {
        return aggregator<T, Base, Literals...>::aggregate(aggregated);
    }
};

// Specialization to terminate agregation when there are no more characters to
// be parsed.
template <typename T, T Base>
struct aggregator<T, Base>
{
    constexpr static T aggregate(T aggregated)
    {
        return aggregated;
    }
};

/**
 * Structures to parse a sequence of characters and convert the result to an
 * integer literal of a desired type.
 *
 * Parsing occurs in two phases. First, any base-specifying prefix is parsed.
 * By default, base-10 is assumed. Specializations are used for other bases.
 * Next, the remaining characters are parsed base on the determined base and
 * converted to the desired type.
 *
 * @tparam T The desired integer literal type.
 * @tparam Base The base of the integer literal being parsed.
 * @tparam Literals Variadic list of characters to parse.
 */
template <typename T, T Base, char... Literals>
struct parser_base
{
    static constexpr T parse()
    {
        return aggregator<T, Base, Literals...>::aggregate(static_cast<T>(0));
    }
};

// Decimal specializations.
template <typename T, char... Literals>
struct parser : parser_base<T, 10, Literals...>
{
};

// Binary specializations.
template <typename T, char... Literals>
struct parser<T, '0', 'b', Literals...> : parser_base<T, 2, Literals...>
{
};

template <typename T, char... Literals>
struct parser<T, '0', 'B', Literals...> : parser_base<T, 2, Literals...>
{
};

// Octal specializations.
template <typename T, char... Literals>
struct parser<T, '0', Literals...> : parser_base<T, 8, Literals...>
{
};

// Hexadecimal specializations.
template <typename T, char... Literals>
struct parser<T, '0', 'x', Literals...> : parser_base<T, 16, Literals...>
{
};

template <typename T, char... Literals>
struct parser<T, '0', 'X', Literals...> : parser_base<T, 16, Literals...>
{
};

/**
 * Validate that a value can be converted to a desired type and perform that
 * conversion
 *
 * @tparam From The current integer literal type.
 * @tparam To The desired integer literal type.
 * @tparam Value The value to convert.

 * @return T The converted integer literal.
 */
template <typename From, typename To, From Value>
constexpr To validate_and_convert()
{
    static_assert(Value >= std::numeric_limits<To>::min(), "Literal overflow");
    static_assert(Value <= std::numeric_limits<To>::max(), "Literal overflow");

    return static_cast<To>(Value);
}

/**
 * Parse a sequence of characters and convert the result to an integer literal
 * of a desired type.
 *
 * @tparam T The desired integer literal type.
 * @tparam Literals Variadic list of characters to parse.
 *
 * @return T The parsed integer literal.
 */
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
