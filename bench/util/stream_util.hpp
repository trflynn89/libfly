#pragma once

#include <iomanip>
#include <locale>
#include <ostream>

namespace fly::benchmark {

/**
 * RAII helper class to make formatting modifications to a stream and ensure those modifications
 * are reset upon destruction.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
class ScopedStreamModifiers
{
public:
    /**
     * Constructor. Store the stream's current state to be restored upon destruction.
     *
     * @param stream The stream to be modified.
     */
    explicit ScopedStreamModifiers(std::ostream &stream);

    /**
     * Destructor. Restore the stream's orginal state.
     */
    ~ScopedStreamModifiers();

    /**
     * Imbue a new locale onto the stream with a specific facet.
     *
     * @tparam Facet The type of facet to imbue.
     */
    template <typename Facet>
    void locale();

    /**
     * Apply an IO manipulator to the stream. See: https://en.cppreference.com/w/cpp/io/manip
     *
     * @tparam Manipulator The type of manipulator to apply.
     *
     * @param manipulator The manipulator to apply.
     */
    template <typename Manipulator>
    void manip(Manipulator &&manipulator);

    /**
     * Modify the number of digits generated for floating point numbers.
     *
     * @param precision The new precision setting.
     */
    void precision(std::streamsize precision);

private:
    ScopedStreamModifiers(const ScopedStreamModifiers &) = delete;
    ScopedStreamModifiers &operator=(const ScopedStreamModifiers &) = delete;

    std::ostream &m_stream;

    const std::locale m_locale;
    const std::ios_base::fmtflags m_flags;
    const std::streamsize m_precision;
};

/**
 * Helper class to center text within a specified width within a steam.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
template <typename StringType>
class Center
{
public:
    /**
     * Constructor.
     *
     * @param width The width to center the text within.
     * @param value The text to be centered.
     */
    Center(std::size_t width, const StringType &value);

    /**
     * Print the centered text.
     *
     * @param stream The stream to print the text into.
     * @param center The instance holding the centering data.
     *
     * @return The same stream object.
     */
    friend std::ostream &operator<<(std::ostream &stream, const Center &center)
    {
        center.print(stream);
        return stream;
    }

private:
    /**
     * Print the centered text.
     *
     * @param stream The stream to print the text into.
     */
    void print(std::ostream &stream) const;

    const std::streamsize m_width;
    const StringType &m_value;
};

/**
 * Locale facet to format numbers with comma separators.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
class CommaPunctuation final : public std::numpunct<std::ios::char_type>
{
protected:
    /**
     * @return The character (comma) to use as the thousands separator.
     */
    std::ios::char_type do_thousands_sep() const override;

    /**
     * @return The number of digits between each pair of thousands separators.
     */
    std::string do_grouping() const override;
};

//==================================================================================================
template <typename Facet>
void ScopedStreamModifiers::locale()
{
    m_stream.imbue({std::locale(), new Facet()});
}

//==================================================================================================
template <typename Manipulator>
void ScopedStreamModifiers::manip(Manipulator &&manipulator)
{
    m_stream << manipulator;
}

//==================================================================================================
template <typename StringType>
Center<StringType>::Center(std::size_t width, const StringType &value) :
    m_width(static_cast<std::streamsize>(width)),
    m_value(value)
{
}

//==================================================================================================
template <typename StringType>
void Center<StringType>::print(std::ostream &stream) const
{
    const std::streamsize length = static_cast<std::streamsize>(m_value.size());

    if (m_width > length)
    {
        const std::streamsize left = (m_width + length) / 2;

        ScopedStreamModifiers scoped_modifiers(stream);
        scoped_modifiers.manip(std::right);

        stream << std::setw(left) << m_value;
        stream << std::setw(m_width - left) << "";
    }
    else
    {
        stream << m_value;
    }
}

} // namespace fly::benchmark
