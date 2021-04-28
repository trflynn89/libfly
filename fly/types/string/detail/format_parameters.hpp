#pragma once

#include "fly/types/string/detail/classifier.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_formatters.hpp"

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

    void (*m_format)(const void *value, FormatContext &context);
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
 * @param context The context holding the formatting state.
 */
template <typename FormatContext, typename T>
void format_user_defined_value(const void *value, FormatContext &context);

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
    template <typename T, fly::enable_if<BasicFormatTraits::is_user_defined<T>> = 0>
    explicit constexpr BasicFormatParameter(const T &value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a type-erased string from any
     * string-like value.
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     */
    template <typename T, fly::enable_if<fly::detail::is_like_supported_string<T>> = 0>
    explicit constexpr BasicFormatParameter(const T &value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a pointer value.
     *
     * @tparam T The pointer type.
     *
     * @param value The pointer value.
     */
    template <typename T, fly::enable_if<BasicFormatTraits::is_pointer<T>> = 0>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store an arithmetic value.
     *
     * @tparam T The arithmetic type.
     *
     * @param value The arithmetic value.
     */
    template <typename T, fly::enable_if<std::is_arithmetic<T>> = 0>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Constructor. Initialize the format parameter to store a default-formatted enumeration value.
     *
     * @tparam T The default-formatted enumeration type.
     *
     * @param value The default-formatted enumeration value.
     */
    template <typename T, fly::enable_if<BasicFormatTraits::is_default_formatted_enum<T>> = 0>
    explicit constexpr BasicFormatParameter(T value) noexcept;

    /**
     * Apply the type-erased formatting function to the stored format parameter.
     *
     * @param context The context holding the formatting state.
     * @param specifier The replacement field to be replaced.
     */
    constexpr void
    format(FormatContext &context, BasicFormatSpecifier<char_type> &&specifier) const;

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
template <typename FormatContext, typename... Parameters>
class BasicFormatParameters
{
    using FormatParameter = BasicFormatParameter<FormatContext>;

public:
    /**
     * Constructor. Type-erase the provided format parameters for storage in an array.
     *
     * @param parameters The format parameters to store.
     */
    constexpr BasicFormatParameters(Parameters &&...parameters) noexcept;

private:
    friend FormatContext;

    const std::array<FormatParameter, sizeof...(Parameters)> m_parameters;
};

/**
 * Create an object that stores an array of formatting parameters.
 *
 * @tparam FormatContext The formatting context type.
 * @tparam ParameterTypes Variadic format parameter types.
 *
 * @param parameters The format parameters to store.
 */
template <typename FormatContext, typename... Parameters>
constexpr inline auto make_format_parameters(Parameters &&...parameters)
{
    return BasicFormatParameters<FormatContext, Parameters...> {
        std::forward<Parameters>(parameters)...};
}

//==================================================================================================
template <typename FormatContext, typename T>
inline void format_user_defined_value(const void *value, FormatContext &context)
{
    Formatter<T, typename FormatContext::char_type>().format(
        *static_cast<const T *>(value),
        context);
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

    Formatter<view_type, typename FormatContext::char_type> formatter(std::move(specifier));

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
    Formatter<T, typename FormatContext::char_type> formatter(std::move(specifier));

    if constexpr (BasicFormatTraits::is_pointer_v<T>)
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
    else if constexpr (std::is_same_v<T, float>)
    {
        formatter.format(value.m_float, context);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        formatter.format(value.m_double, context);
    }
    else if constexpr (std::is_same_v<T, long double>)
    {
        formatter.format(value.m_long_double, context);
    }
    else if constexpr (std::is_same_v<T, bool>)
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
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter() noexcept :
    m_type(Type::Invalid),
    m_value {.m_monostate {}}
{
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<BasicFormatTraits::is_user_defined<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(const T &value) noexcept
    :
    m_type(Type::UserDefined),
    m_value {.m_user_defined {&value, format_user_defined_value<FormatContext, T>}}
{
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<fly::detail::is_like_supported_string<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(const T &value) noexcept
    :
    m_type(Type::String)
{
    using U = std::remove_cvref_t<T>;

    using string_like_type = fly::detail::is_like_supported_string_t<T>;
    using char_like_type = typename string_like_type::value_type;
    using view_like_type = typename fly::detail::BasicStringTraits<char_like_type>::view_type;

    view_like_type view;

    if constexpr (std::is_array_v<U> || std::is_pointer_v<U>)
    {
        view = view_like_type(value, BasicClassifier<char_like_type>::size(value));
    }
    else
    {
        view = view_like_type(value);
    }

    m_value.m_string = {
        static_cast<const void *>(view.data()),
        view.size(),
        format_string_value<FormatContext, char_like_type>};
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<BasicFormatTraits::is_pointer<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept :
    m_type(Type::Pointer),
    m_value {.m_standard {.m_pointer {value}, .m_format {format_standard_value<FormatContext, T>}}}
{
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<std::is_arithmetic<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept
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
    else if constexpr (std::is_same_v<T, bool>)
    {
        m_type = Type::Bool;
        m_value.m_standard.m_bool = value;
    }
    else if constexpr (std::is_signed_v<T>)
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
template <typename T, fly::enable_if<BasicFormatTraits::is_default_formatted_enum<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept :
    BasicFormatParameter(static_cast<std::underlying_type_t<T>>(value))
{
}

//==================================================================================================
template <typename FormatContext>
constexpr inline void BasicFormatParameter<FormatContext>::format(
    FormatContext &context,
    BasicFormatSpecifier<char_type> &&specifier) const
{
    switch (m_type)
    {
        case Type::UserDefined:
            m_value.m_user_defined.m_format(m_value.m_user_defined.m_value, context);
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
constexpr inline auto BasicFormatParameter<FormatContext>::visit(Visitor &&visitor) const
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
template <typename FormatContext, typename... Parameters>
constexpr inline BasicFormatParameters<FormatContext, Parameters...>::BasicFormatParameters(
    Parameters &&...parameters) noexcept :
    m_parameters {FormatParameter {std::forward<Parameters>(parameters)}...}
{
}

} // namespace fly::detail
