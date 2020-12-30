#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace fly::detail {

/**
 * Structures to recursively aggregate the value of an integer literal as it is being parsed.
 *
 * @tparam T The desired integer literal type.
 * @tparam Base The base of the integer literal being parsed.
 * @tparam Literals Variadic list of characters to parse.
 */
template <typename T, T Base, char... Literals>
struct Aggregator;

// Specialization to convert the current character being parsed to an integer and recurse on the
// remainder of the characters.
template <typename T, T Base, char Digit, char... Literals>
struct Aggregator<T, Base, Digit, Literals...>
{
    static constexpr T aggregate(T aggregated)
    {
        return Aggregator<T, Base, Literals...>::aggregate(
            aggregated * Base + parse_and_validate_literal());
    }

private:
    /**
     * Convert the current character being parsed to an integer literal. Validate that the character
     * fits within its base.
     *
     * @return The result of conversion.
     */
    static constexpr T parse_and_validate_literal()
    {
        constexpr bool literal_is_valid_for_base =
            ((Base == 2) && ((Digit >= '0') && (Digit <= '1'))) ||
            ((Base == 8) && ((Digit >= '0') && (Digit <= '7'))) ||
            ((Base == 10) && ((Digit >= '0') && (Digit <= '9'))) ||
            ((Base == 16) && ((Digit >= '0') && (Digit <= '9'))) ||
            ((Base == 16) && ((Digit >= 'A') && (Digit <= 'F'))) ||
            ((Base == 16) && ((Digit >= 'a') && (Digit <= 'f')));
        static_assert(literal_is_valid_for_base, "Invalid literal for base");

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
            return static_cast<T>(Digit) - static_cast<T>('0');
        }
    }
};

// Specialization to simply ignore a separator character and recurse on the remainder of the
// characters.
template <typename T, T Base, char... Literals>
struct Aggregator<T, Base, '\'', Literals...>
{
    static constexpr T aggregate(T aggregated)
    {
        return Aggregator<T, Base, Literals...>::aggregate(aggregated);
    }
};

// Specialization to terminate agregation when there are no more characters to be parsed.
template <typename T, T Base>
struct Aggregator<T, Base>
{
    static constexpr T aggregate(T aggregated)
    {
        return aggregated;
    }
};

/**
 * Structures to parse a sequence of characters and convert the result to an integer literal of a
 * desired type.
 *
 * Parsing occurs in two phases. First, any base-specifying prefix is parsed. By default, base-10 is
 * assumed, and specializations are used for other bases. Next, the remaining characters are parsed
 * base on the determined base and converted to the desired type.
 *
 * @tparam T The desired integer literal type.
 * @tparam Base The base of the integer literal being parsed.
 * @tparam Literals Variadic list of characters to parse.
 */
template <typename T, T Base, char... Literals>
struct ParserBase
{
    static constexpr T parse()
    {
        return Aggregator<T, Base, Literals...>::aggregate(static_cast<T>(0));
    }
};

// Decimal default.
template <typename T, char... Literals>
struct Parser : ParserBase<T, 10, Literals...>
{
};

// Binary specializations.
template <typename T, char... Literals>
struct Parser<T, '0', 'b', Literals...> : ParserBase<T, 2, Literals...>
{
};

template <typename T, char... Literals>
struct Parser<T, '0', 'B', Literals...> : ParserBase<T, 2, Literals...>
{
};

// Octal specializations.
template <typename T, char... Literals>
struct Parser<T, '0', Literals...> : ParserBase<T, 8, Literals...>
{
};

// Hexadecimal specializations.
template <typename T, char... Literals>
struct Parser<T, '0', 'x', Literals...> : ParserBase<T, 16, Literals...>
{
};

template <typename T, char... Literals>
struct Parser<T, '0', 'X', Literals...> : ParserBase<T, 16, Literals...>
{
};

/**
 * Validate that a value can be converted to a desired type and perform that conversion
 *
 * @tparam From The current integer literal type.
 * @tparam To The desired integer literal type.
 * @tparam Value The value to convert.
 *
 * @return The converted integer literal.
 */
template <typename From, typename To, From Value>
constexpr To validate_and_convert()
{
    static_assert(Value >= std::numeric_limits<To>::min(), "Literal overflow");
    static_assert(Value <= std::numeric_limits<To>::max(), "Literal overflow");

    return static_cast<To>(Value);
}

/**
 * Parse a sequence of characters and convert the result to an integer literal of a desired type.
 *
 * @tparam T The desired integer literal type.
 * @tparam Literals Variadic list of characters to parse.
 *
 * @return The parsed integer literal.
 */
template <typename T, char... Literals>
constexpr T literal()
{
    using ValueType = std::conditional_t<std::is_signed_v<T>, std::intmax_t, std::uintmax_t>;
    return validate_and_convert<ValueType, T, Parser<ValueType, Literals...>::parse()>();
}

} // namespace fly::detail
