#pragma once

#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_format_specifier.hpp"
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

    void (*m_format)(const void *value, std::size_t size, FormatContext &context);
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
 */
template <typename FormatContext, typename T>
void format_string_value(const void *value, std::size_t size, FormatContext &context);

/**
 * A container to hold a single type-erased format parameter.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename FormatContext>
class BasicFormatParameter
{
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
    template <
        typename T,
        fly::disable_if_any<
            detail::is_like_supported_string<T>,
            BasicFormatTraits::is_pointer<T>,
            std::is_arithmetic<T>,
            BasicFormatTraits::is_default_formatted_enum<T>> = 0>
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
        Generic,
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
        UserDefinedValue<FormatContext> m_generic;
        StringValue<FormatContext> m_string;
        const void *m_pointer;
        std::int64_t m_signed_int;
        std::uint64_t m_unsigned_int;
        float m_float;
        double m_double;
        long double m_long_double;
        bool m_bool;
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
 * Provides access to the formatting state consisting of the format parameters, replacement fields
 * and the output iterator.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename OutputIterator, typename CharType>
class BasicFormatContext
{
    using FormatParameter = BasicFormatParameter<BasicFormatContext>;
    using FormatSpecifier = BasicFormatSpecifier<CharType>;

public:
    using char_type = CharType;

    /**
     * Constructor.
     *
     * @param out The output iterator into which the formatted value should be written.
     * @param parameters The format parameters created with |make_format_parameters|.
     */
    template <typename... Parameters>
    constexpr BasicFormatContext(
        OutputIterator out,
        const BasicFormatParameters<BasicFormatContext, Parameters...> &parameters) noexcept;

    /**
     * Get the object holding the format parameter at the specified index. If the index is invalid,
     * returns a format parameter holding |MonoState|.
     *
     * @param index The index to lookup.
     *
     * @return The format parameter at the specified index.
     */
    FormatParameter arg(std::size_t index) const;

    /**
     * @return The output iterator into which the formatted value should be written.
     */
    OutputIterator &out();

    /**
     * @return The formatting replacement field currently being used for formatting.
     */
    FormatSpecifier &spec();

private:
    BasicFormatContext(const BasicFormatContext &) = delete;
    BasicFormatContext &operator=(const BasicFormatContext &) = delete;

    OutputIterator m_out;

    const FormatParameter *m_parameters;
    const std::size_t m_parameters_size;

    FormatSpecifier m_specifier {};
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
inline void format_string_value(const void *value, std::size_t size, FormatContext &context)
{
    static_assert(detail::is_supported_character_v<T>);

    std::basic_string_view<T> view(static_cast<const T *>(value), size);
    Formatter<decltype(view), typename FormatContext::char_type>().format(view, context);
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
template <
    typename T,
    fly::disable_if_any<
        detail::is_like_supported_string<T>,
        BasicFormatTraits::is_pointer<T>,
        std::is_arithmetic<T>,
        BasicFormatTraits::is_default_formatted_enum<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(const T &value) noexcept
    :
    m_type(Type::Generic),
    m_value {.m_generic {&value, format_user_defined_value<FormatContext, T>}}
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

    using string_type = fly::detail::is_like_supported_string_t<T>;
    using char_type = typename fly::detail::BasicStringTraits<string_type>::char_type;
    using view_type = typename fly::detail::BasicStringTraits<string_type>::view_type;

    view_type view;

    if constexpr (std::is_array_v<U> || std::is_pointer_v<U>)
    {
        view = view_type(value, fly::detail::BasicStringClassifier<string_type>::size(value));
    }
    else
    {
        view = view_type(value);
    }

    m_value.m_string = {
        static_cast<const void *>(view.data()),
        view.size(),
        format_string_value<FormatContext, char_type>};
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<BasicFormatTraits::is_pointer<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept :
    m_type(Type::Pointer),
    m_value {.m_pointer {value}}
{
}

//==================================================================================================
template <typename FormatContext>
template <typename T, fly::enable_if<std::is_arithmetic<T>>>
constexpr inline BasicFormatParameter<FormatContext>::BasicFormatParameter(T value) noexcept
{
    if constexpr (std::is_floating_point_v<T>)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            m_type = Type::Float;
            m_value.m_float = value;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            m_type = Type::Double;
            m_value.m_double = value;
        }
        else if constexpr (std::is_same_v<T, long double>)
        {
            m_type = Type::LongDouble;
            m_value.m_long_double = value;
        }
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        m_type = Type::Bool;
        m_value.m_bool = value;
    }
    else if constexpr (std::is_signed_v<T>)
    {
        m_type = Type::SignedInt;
        m_value.m_signed_int = value;
    }
    else
    {
        m_type = Type::UnsignedInt;
        m_value.m_unsigned_int = value;
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
template <typename Visitor>
constexpr inline auto BasicFormatParameter<FormatContext>::visit(Visitor &&visitor) const
{
    switch (m_type)
    {
        case Type::Generic:
            return visitor(m_value.m_generic);
        case Type::String:
            return visitor(m_value.m_string);
        case Type::Pointer:
            return visitor(m_value.m_pointer);
        case Type::SignedInt:
            return visitor(m_value.m_signed_int);
        case Type::UnsignedInt:
            return visitor(m_value.m_unsigned_int);
        case Type::Float:
            return visitor(m_value.m_float);
        case Type::Double:
            return visitor(m_value.m_double);
        case Type::LongDouble:
            return visitor(m_value.m_long_double);
        case Type::Bool:
            return visitor(m_value.m_bool);
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

//==================================================================================================
template <typename OutputIterator, typename CharType>
template <typename... Parameters>
constexpr inline BasicFormatContext<OutputIterator, CharType>::BasicFormatContext(
    OutputIterator out,
    const BasicFormatParameters<BasicFormatContext, Parameters...> &parameters) noexcept :
    m_out(std::move(out)),
    m_parameters(parameters.m_parameters.data()),
    m_parameters_size(parameters.m_parameters.size())
{
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline auto BasicFormatContext<OutputIterator, CharType>::arg(std::size_t index) const
    -> FormatParameter
{
    if (index < m_parameters_size)
    {
        return m_parameters[index];
    }

    return {};
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline OutputIterator &BasicFormatContext<OutputIterator, CharType>::out()
{
    return m_out;
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline auto BasicFormatContext<OutputIterator, CharType>::spec() -> FormatSpecifier &
{
    return m_specifier;
}

} // namespace fly::detail
