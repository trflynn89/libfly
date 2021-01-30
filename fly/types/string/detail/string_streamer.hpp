#pragma once

#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/string_unicode.hpp"
#include "fly/types/string/string_literal.hpp"

#include <cctype>
#include <cstdint>
#include <ios>
#include <locale>
#include <type_traits>

namespace fly::detail {

/**
 * Helper struct to stream generic values into a std::basic_ostream.
 *
 * For std::string and std::wstring, the "normal" stream types are used (std::ostream and
 * std::wostream, and their children, respectively).
 *
 * For std::u8string, std::u16string, and std::u32string, the STL does not provide stream types. For
 * a general solution, std::ostream is used, and any string value is converted to UTF-8.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType>
struct BasicStringStreamer
{
    using traits = BasicStringTraits<StringType>;

    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamed_type;
    using streamed_char_type = typename traits::streamed_char_type;

    /**
     * Stream the given value into the given stream.
     *
     * For all string-like types, if the type corresponds to the stream type, then the string is
     * streamed as-is. Other string-like types are converted to the Unicode encoding used by the
     * stream type.
     *
     * For all character types, the character is case to the character type used by the stream type.
     *
     * For any other type which has an operator<< overload defined, the value is streamed using that
     * overload.
     *
     * All other types are dropped.
     *
     * @tparam T The type of the value to stream.
     *
     * @param stream The stream to insert the value into.
     * @param value The value to stream.
     */
    template <typename T>
    static void stream_value(ostream_type &stream, T &&value);

private:
    /**
     * Stream a string-like value into the given stream. If the type corresponds to the stream type,
     * then the string is streamed as-is. Other string-like types are converted to the Unicode
     * encoding used by the stream type.
     *
     * @tparam T The type of the string-like value to stream.
     *
     * @param stream The stream to insert the value into.
     * @param value The string-like value to stream.
     */
    template <typename T>
    static void stream_string(ostream_type &stream, T &&value);
};

/**
 * RAII helper class to make formatting modifications to a stream and ensure those modifications
 * are reset upon destruction.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType>
class BasicStreamModifiers
{
    using traits = BasicStringTraits<StringType>;

    using ostream_type = typename traits::ostream_type;
    using streamed_char_type = typename traits::streamed_char_type;

public:
    /**
     * Constructor. Store the stream's current state to be restored upon destruction.
     *
     * @param stream The stream to be modified.
     */
    explicit BasicStreamModifiers(ostream_type &stream) noexcept;

    /**
     * Destructor. Restore the stream's orginal state.
     */
    ~BasicStreamModifiers();

    /**
     * Sets a formatting flag on the stream.
     *
     * @param flag The new formatting setting.
     */
    void setf(std::ios_base::fmtflags flag);

    /**
     * Clears a mask of formatting flags on the stream and sets a specific flag.
     *
     * @param flag The new formatting setting.
     * @param mask The formatting mask to clear.
     */
    void setf(std::ios_base::fmtflags flag, std::ios_base::fmtflags mask);

    /**
     * Imbue a new locale onto the stream with a specific facet.
     *
     * @tparam Facet The type of facet to imbue.
     */
    template <typename Facet>
    void locale();

    /**
     * Set the fill character of the stream.
     *
     * @param ch The new fill setting.
     */
    void fill(streamed_char_type ch);

    /**
     * Set the width of the stream.
     *
     * @param size The new width setting.
     */
    void width(std::streamsize size);

    /**
     * Set the precision of the stream.
     *
     * @param size The new precision setting.
     */
    void precision(std::streamsize size);

private:
    BasicStreamModifiers(const BasicStreamModifiers &) = delete;
    BasicStreamModifiers &operator=(const BasicStreamModifiers &) = delete;

    ostream_type &m_stream;

    const std::ios_base::fmtflags m_flags;
    bool m_changed_flags {false};

    const std::locale m_locale;
    bool m_changed_locale {false};

    const streamed_char_type m_fill;
    bool m_changed_fill {false};

    const std::streamsize m_width;
    bool m_changed_width {false};

    const std::streamsize m_precision;
    bool m_changed_precision {false};
};

/**
 * Helper facet to support BasicFormatSpecifier::Sign::NegativeOnlyWithPositivePadding. Overrides
 * std::ctype to replace the positive sign character with a space.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename CharType>
class PositivePaddingFacet : public std::ctype<CharType>
{
protected:
    CharType do_widen(char ch) const override;

    const char *do_widen(const char *begin, const char *end, CharType *dest) const override;

private:
    static constexpr const auto s_plus_sign = FLY_CHR(char, '+');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
};

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicStringStreamer<StringType>::stream_value(ostream_type &stream, T &&value)
{
    using U = std::remove_cvref_t<T>;

    if constexpr (detail::is_like_supported_string_v<U>)
    {
        stream_string(stream, std::forward<T>(value));
    }
    else if constexpr (detail::is_supported_character_v<T>)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        stream << static_cast<streamed_char_type>(std::forward<T>(value));
    }
    else if constexpr (traits::OstreamTraits::template is_declared_v<U>)
    {
        stream << std::forward<T>(value);
    }
}

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicStringStreamer<StringType>::stream_string(ostream_type &stream, T &&value)
{
    using string_like_type = detail::is_like_supported_string_t<T>;
    using string_like_traits = BasicStringTraits<string_like_type>;

    if constexpr (std::is_same_v<streamed_type, string_like_type>)
    {
        stream << value;
    }
    else
    {
        using unicode = BasicStringUnicode<string_like_type>;
        using streamer = BasicStringStreamer<streamed_type>;

        typename string_like_traits::view_type view(std::forward<T>(value));
        auto it = view.cbegin();
        const auto end = view.cend();

        if (auto converted = unicode::template convert_encoding<streamed_type>(it, end); converted)
        {
            streamer::stream_value(stream, *std::move(converted));
        }
    }
}

//==================================================================================================
template <typename StringType>
BasicStreamModifiers<StringType>::BasicStreamModifiers(ostream_type &stream) noexcept :
    m_stream(stream),
    m_flags(stream.flags()),
    m_locale(stream.getloc()),
    m_fill(stream.fill()),
    m_width(stream.width()),
    m_precision(stream.precision())
{
}

//==================================================================================================
template <typename StringType>
BasicStreamModifiers<StringType>::~BasicStreamModifiers()
{
    if (m_changed_flags)
    {
        m_stream.flags(m_flags);
    }
    if (m_changed_locale)
    {
        m_stream.imbue(m_locale);
    }
    if (m_changed_fill)
    {
        m_stream.fill(m_fill);
    }
    if (m_changed_width)
    {
        m_stream.width(m_width);
    }
    if (m_changed_precision)
    {
        m_stream.precision(m_precision);
    }
}

//==================================================================================================
template <typename StringType>
inline void BasicStreamModifiers<StringType>::setf(std::ios_base::fmtflags flag)
{
    m_stream.setf(flag);
    m_changed_flags = true;
}

//==================================================================================================
template <typename StringType>
inline void
BasicStreamModifiers<StringType>::setf(std::ios_base::fmtflags flag, std::ios_base::fmtflags mask)
{
    m_stream.setf(flag, mask);
    m_changed_flags = true;
}

//==================================================================================================
template <typename StringType>
template <typename Facet>
inline void BasicStreamModifiers<StringType>::locale()
{
    m_stream.imbue({m_stream.getloc(), new Facet()});
    m_changed_locale = true;
}

//==================================================================================================
template <typename StringType>
inline void BasicStreamModifiers<StringType>::fill(streamed_char_type ch)
{
    m_stream.fill(ch);
    m_changed_fill = true;
}

//==================================================================================================
template <typename StringType>
inline void BasicStreamModifiers<StringType>::width(std::streamsize size)
{
    m_stream.width(size);
    m_changed_width = true;
}

//==================================================================================================
template <typename StringType>
inline void BasicStreamModifiers<StringType>::precision(std::streamsize size)
{
    m_stream.precision(size);
    m_changed_precision = true;
}

//==================================================================================================
template <typename CharType>
CharType PositivePaddingFacet<CharType>::do_widen(char ch) const
{
    return (ch == s_plus_sign) ? s_space : static_cast<CharType>(ch);
}

//==================================================================================================
template <typename CharType>
const char *
PositivePaddingFacet<CharType>::do_widen(const char *begin, const char *end, CharType *dest) const
{
    while (begin != end)
    {
        *dest++ = do_widen(*begin++);
    }

    return end;
}

} // namespace fly::detail
