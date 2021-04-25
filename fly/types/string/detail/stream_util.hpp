#pragma once

#include "fly/types/string/literals.hpp"

#include <cctype>
#include <ios>
#include <locale>
#include <ostream>

namespace fly::detail {

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
public:
    /**
     * Constructor. Store the stream's current state to be restored upon destruction.
     *
     * @param stream The stream to be modified.
     */
    explicit BasicStreamModifiers(std::ostream &stream) noexcept;

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
    void fill(char ch);

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

    std::ostream &m_stream;

    const std::ios_base::fmtflags m_flags;
    bool m_changed_flags {false};

    const std::locale m_locale;
    bool m_changed_locale {false};

    const char m_fill;
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
BasicStreamModifiers<StringType>::BasicStreamModifiers(std::ostream &stream) noexcept :
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
inline void BasicStreamModifiers<StringType>::fill(char ch)
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
