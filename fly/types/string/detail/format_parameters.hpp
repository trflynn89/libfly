#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/string/detail/classifier.hpp"
#include "fly/types/string/detail/format_parse_context.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/string_concepts.hpp"
#include "fly/types/string/formatters.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace fly::detail {

/**
 * Empty placeholder structure used for an invalid formatting parameter state.
 */
struct MonoState
{
};

/**
 * Structure to store a type-erased user-defined object.
 */
template <typename FormatContext>
struct UserDefinedValue
{
    const void *m_value;

    void (*m_format)(
        const void *value,
        BasicFormatParseContext<typename FormatContext::char_type> &,
        FormatContext &context,
        BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);
};

/**
 * Structure to store a type-erased string-like object. May be used for strings with any character
 * encoding.
 */
template <typename FormatContext>
struct StringValue
{
    const void *m_value;
    std::size_t m_size;

    void (*m_format)(
        const void *value,
        std::size_t size,
        FormatContext &context,
        BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);
};

/**
 * Structure to store a type-erased standard value.
 */
template <typename FormatContext>
struct StandardValue
{
    union
    {
        const void *m_pointer;
        std::int64_t m_signed_int;
        std::uint64_t m_unsigned_int;
        float m_float;
        double m_double;
        long double m_long_double;
        bool m_bool;
    };

    void (*m_format)(
        StandardValue value,
        FormatContext &context,
        BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);
};

/**
 * Re-form a type-erased user-defined value and format that value.
 *
 * @tparam FormatContext The formatting context type.
 * @tparam T The user-defined type.
 *
 * @param value A pointer to the type-erased user-defined value to format.
 * @param parse_context The context holding the format parsing state.
 * @param context The context holding the formatting state.
 * @param specifier The replacement field to be replaced.
 */
template <typename FormatContext, typename T>
void format_user_defined_value(
    const void *value,
    BasicFormatParseContext<typename FormatContext::char_type> &parse_context,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);

/**
 * Re-form a type-erased string value and format that value.
 *
 * @tparam FormatContext The formatting context type.
 * @tparam T The string-like type.
 *
 * @param value A pointer to the type-erased string to format.
 * @param size The size of the string to format.
 * @param context The context holding the formatting state.
 * @param specifier The replacement field to be replaced.
 */
template <typename FormatContext, typename T>
void format_string_value(
    const void *value,
    std::size_t size,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);

/**
 * Re-form a type-erased standard value and format that value.
 *
 * @tparam FormatContext The formatting context type.
 * @tparam T The standrd type.
 *
 * @param value The container holding the type-erased value.
 * @param context The context holding the formatting state.
 * @param specifier The replacement field to be replaced.
 */
template <typename FormatContext, typename T>
void format_standard_value(
    StandardValue<FormatContext> value,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier);

/**
 * A container to hold a single type-erased format parameter.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename FormatContext>
class BasicFormatParameter
{
    using char_type = typename FormatContext::char_type;

public:
    /**
     * Constructor. Initialize the format parameter to an invalid state.
     */
    constexpr BasicFormatParameter() noexcept;

    /**
     * Constructor. Initialize the format parameter to store a type-erased user-defined value.
     *
     * @tparam T The user-defined type.
     *
     * @param value The user-defined value.
     */
    template <FormattableUserDefined T>
    explicit constexpr BasicFormatParameter(const T &value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a type-erased string from any
     * string-like value.
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     */
    template <FormattableString T>
    explicit constexpr BasicFormatParameter(const T &value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a pointer value.
     *
     * @tparam T The pointer type.
     *
     * @param value The pointer value.
     */
    template <FormattablePointer T>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store an integral value.
     *
     * @tparam T The integral type.
     *
     * @param value The integral value.
     */
    template <FormattableIntegral T>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a floating-point value.
     *
     * @tparam T The floating-point type.
     *
     * @param value The floating-point value.
     */
    template <FormattableFloatingPoint T>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a boolean value.
     *
     * @tparam T The boolean type.
     *
     * @param value The boolean value.
     */
    template <FormattableBoolean T>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Apply the type-erased formatting function to the stored format parameter.
     *
     * @param parse_context The context holding the format parsing state.
     * @param context The context holding the formatting state.
     * @param specifier The replacement field to be replaced.
     */
    constexpr void format(
        BasicFormatParseContext<typename FormatContext::char_type> &parse_context,
        FormatContext &context,
        BasicFormatSpecifier<char_type> &&specifier) const;

    /**
     * Apply the provided visitor to the stored format parameter.
     *
     * @tparam Visitor Type of the visitor to invoke.
     *
     * @param visitor The visitor to invoke.
     */
    template <typename Visitor>
    constexpr auto visit(Visitor &&visitor) const;

    /**
     * @return True if this format parameter is holding a valid type.
     */
    explicit operator bool() const noexcept;

private:
    enum class Type : std::uint8_t
    {
        Invalid,
        UserDefined,
        String,
        Pointer,
        SignedInt,
        UnsignedInt,
        Float,
        Double,
        LongDouble,
        Bool,
    };

    union Value
    {
        MonoState m_monostate;
        UserDefinedValue<FormatContext> m_user_defined;
        StringValue<FormatContext> m_string;
        StandardValue<FormatContext> m_standard;
    };

    Type m_type;
    Value m_value;
};

/**
 * A container to hold type-erased variadic format parameters.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename FormatContext, typename... ParameterTypes>
class BasicFormatParameters
{
    using FormatParameter = BasicFormatParameter<FormatContext>;

public:
    /**
     * Constructor. Type-erase the provided format parameters for storage in an array.
     *
     * @param parameters The format parameters to store.
     */
    constexpr BasicFormatParameters(ParameterTypes &&...parameters) noexcept;

private:
    friend FormatContext;

    const std::array<FormatParameter, sizeof...(ParameterTypes)> m_parameters;
};

/**
 * Create an object that stores an array of formatting parameters.
 *
 * @tparam FormatContext The formatting context type.
 * @tparam ParameterTypes Variadic format parameter types.
 *
 * @param parameters The format parameters to store.
 */
template <typename FormatContext, Formattable<FormatContext>... ParameterTypes>
constexpr auto make_format_parameters(ParameterTypes &&...parameters)
{
    return BasicFormatParameters<FormatContext, ParameterTypes...> {
        std::forward<ParameterTypes>(parameters)...};
}

//==================================================================================================
template <typename FormatContext, typename T>
inline void format_user_defined_value(
    const void *value,
    BasicFormatParseContext<typename FormatContext::char_type> &parse_context,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier)
{
    using Formatter = typename FormatContext::template formatter_type<T>;

    Formatter formatter;
    parse_context.lexer().set_position(specifier.m_parse_index);

    if constexpr (FormattableWithParsing<decltype(parse_context), Formatter>)
    {
        formatter.parse(parse_context);
    }
    else
    {
        if (!parse_context.lexer().consume_if(FLY_CHR(typename FormatContext::char_type, '}')))
        {
            parse_context.on_error(
                "User-defined formatter without a parse method may not have formatting options");
        }
    }

    if (!parse_context.has_error())
    {
        formatter.format(*static_cast<const T *>(value), context);
    }
}

//==================================================================================================
template <typename FormatContext, typename T>
inline void format_string_value(
    const void *value,
    std::size_t size,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier)
{
    using view_type = std::basic_string_view<T>;

    typename FormatContext::template formatter_type<view_type> formatter(std::move(specifier));

    view_type view(static_cast<const T *>(value), size);
    formatter.format(view, context);
}

//==================================================================================================
template <typename FormatContext, typename T>
inline void format_standard_value(
    StandardValue<FormatContext> value,
    FormatContext &context,
    BasicFormatSpecifier<typename FormatContext::char_type> &&specifier)
{
    typename FormatContext::template formatter_type<T> formatter(std::move(specifier));

    if constexpr (FormattablePointer<T>)
    {
        if constexpr (std::is_null_pointer_v<T>)
        {
            formatter.format(nullptr, context);
        }
        else if constexpr (std::is_const_v<T>)
        {
            formatter.format(static_cast<T>(value.m_pointer), context);
        }
        else
        {
            formatter.format(static_cast<T>(const_cast<void *>(value.m_pointer)), context);
        }
    }
    else if constexpr (fly::SameAs<T, float>)
    {
        formatter.format(value.m_float, context);
    }
    else if constexpr (fly::SameAs<T, double>)
    {
        formatter.format(value.m_double, context);
    }
    else if constexpr (fly::SameAs<T, long double>)
    {
        formatter.format(value.m_long_double, context);
    }
    else if constexpr (fly::SameAs<T, bool>)
    {
        formatter.format(value.m_bool, context);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        formatter.format(static_cast<T>(value.m_signed_int), context);
    }
    else
    {
        formatter.format(static_cast<T>(value.m_unsigned_int), context);
    }
}

//==================================================================================================
template <typename FormatContext>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter() noexcept :
    m_type(Type::Invalid),
    m_value {.m_monostate {}}
{
}

//==================================================================================================
template <typename FormatContext>
template <FormattableUserDefined T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(const T &value) noexcept :
    m_type(Type::UserDefined),
    m_value {.m_user_defined {&value, format_user_defined_value<FormatContext, T>}}
{
}

//==================================================================================================
template <typename FormatContext>
template <FormattableString T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(const T &value) noexcept :
    m_type(Type::String)
{
    using U = std::remove_cvref_t<T>;

    using standard_character_type = detail::StandardCharacterType<T>;
    using standard_view_type = std::basic_string_view<standard_character_type>;

    standard_view_type view;

    if constexpr (std::is_array_v<U> || std::is_pointer_v<U>)
    {
        view = standard_view_type(value, BasicClassifier<standard_character_type>::size(value));
    }
    else
    {
        view = standard_view_type(value);
    }

    m_value.m_string = {
        static_cast<const void *>(view.data()),
        view.size(),
        format_string_value<FormatContext, standard_character_type>};
}

//==================================================================================================
template <typename FormatContext>
template <FormattablePointer T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept :
    m_type(Type::Pointer),
    m_value {.m_standard {.m_pointer {value}, .m_format {format_standard_value<FormatContext, T>}}}
{
}

//==================================================================================================
template <typename FormatContext>
template <FormattableIntegral T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept
{
    m_value.m_standard.m_format = format_standard_value<FormatContext, T>;

    if constexpr (std::is_signed_v<T>)
    {
        m_type = Type::SignedInt;
        m_value.m_standard.m_signed_int = value;
    }
    else
    {
        m_type = Type::UnsignedInt;
        m_value.m_standard.m_unsigned_int = value;
    }
}

//==================================================================================================
template <typename FormatContext>
template <FormattableFloatingPoint T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept
{
    m_value.m_standard.m_format = format_standard_value<FormatContext, T>;

    if constexpr (std::is_same_v<T, float>)
    {
        m_type = Type::Float;
        m_value.m_standard.m_float = value;
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        m_type = Type::Double;
        m_value.m_standard.m_double = value;
    }
    else if constexpr (std::is_same_v<T, long double>)
    {
        m_type = Type::LongDouble;
        m_value.m_standard.m_long_double = value;
    }
}

//==================================================================================================
template <typename FormatContext>
template <FormattableBoolean T>
constexpr BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept :
    m_type(Type::Bool),
    m_value {.m_standard {.m_bool {value}, .m_format {format_standard_value<FormatContext, T>}}}
{
}

//==================================================================================================
template <typename FormatContext>
constexpr void BasicFormatParameter<FormatContext>::format(
    BasicFormatParseContext<typename FormatContext::char_type> &parse_context,
    FormatContext &context,
    BasicFormatSpecifier<char_type> &&specifier) const
{
    switch (m_type)
    {
        case Type::UserDefined:
            m_value.m_user_defined.m_format(
                m_value.m_user_defined.m_value,
                parse_context,
                context,
                std::move(specifier));
            break;
        case Type::String:
            m_value.m_string.m_format(
                m_value.m_string.m_value,
                m_value.m_string.m_size,
                context,
                std::move(specifier));
            break;
        case Type::Pointer:
        case Type::SignedInt:
        case Type::UnsignedInt:
        case Type::Float:
        case Type::Double:
        case Type::LongDouble:
        case Type::Bool:
            m_value.m_standard.m_format(m_value.m_standard, context, std::move(specifier));
            break;
        default:
            break;
    }
}

//==================================================================================================
template <typename FormatContext>
template <typename Visitor>
constexpr auto BasicFormatParameter<FormatContext>::visit(Visitor &&visitor) const
{
    switch (m_type)
    {
        case Type::UserDefined:
            return visitor(m_value.m_user_defined);
        case Type::String:
            return visitor(m_value.m_string);
        case Type::Pointer:
            return visitor(m_value.m_standard.m_pointer);
        case Type::SignedInt:
            return visitor(m_value.m_standard.m_signed_int);
        case Type::UnsignedInt:
            return visitor(m_value.m_standard.m_unsigned_int);
        case Type::Float:
            return visitor(m_value.m_standard.m_float);
        case Type::Double:
            return visitor(m_value.m_standard.m_double);
        case Type::LongDouble:
            return visitor(m_value.m_standard.m_long_double);
        case Type::Bool:
            return visitor(m_value.m_standard.m_bool);
        default:
            return visitor(m_value.m_monostate);
    }
}

//==================================================================================================
template <typename FormatContext>
inline BasicFormatParameter<FormatContext>::operator bool() const noexcept
{
    return m_type != Type::Invalid;
}

//==================================================================================================
template <typename FormatContext, typename... ParameterTypes>
constexpr BasicFormatParameters<FormatContext, ParameterTypes...>::BasicFormatParameters(
    ParameterTypes &&...parameters) noexcept :
    m_parameters {FormatParameter {std::forward<ParameterTypes>(parameters)}...}
{
}

} // namespace fly::detail
