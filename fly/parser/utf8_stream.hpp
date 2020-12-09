#pragma once

#include "fly/fly.hpp"
#include "fly/traits/traits.hpp"

#include <istream>
#include <memory>
#include <streambuf>
#include <string>
#include <type_traits>

namespace fly {

/**
 * A wrapper around std::basic_istream to hide the templated stream character type. This mostly
 * exists so that concrete parsers do not need to be templated based on that character type.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 29, 2020
 */
class UTF8Stream
{
public:
    /**
     * Determine if a character type is supported by the UTF8Stream class.
     *
     * @tparam CharType The character type to check.
     *
     * @return True if the character type is supported.
     */
    template <typename CharType>
    inline static constexpr bool supports_utf8_stream()
    {
        if constexpr (is_macos())
        {
            // Apple Clang does not yet have standard library support for char8_t:
            // https://en.cppreference.com/w/cpp/compiler_support
            return std::is_same_v<CharType, char>;
        }
        else
        {
            return any_same_v<CharType, char, char8_t>;
        }
    }

    /**
     * Create a UTF8Stream wrapped around an existing std::basic_istream. It is expected that the
     * existing stream outlive this wrapper instance.
     *
     * @tparam CharType The character type for the std::basic_istream. Must be char or char8_t.
     *
     * @param stream The underlying stream to wrap.
     *
     * @return The wrapper UTF8Stream instance.
     */
    template <typename CharType>
    static std::unique_ptr<UTF8Stream> create(std::basic_istream<CharType> &stream);

    /**
     * Destructor.
     */
    virtual ~UTF8Stream() = default;

    /**
     * Read the next character from the stream without extracting it.
     *
     * @tparam IntType The type to cast the character to.
     *
     * @return The peeked character.
     */
    template <typename IntType = int>
    IntType peek();

    /**
     * Read the next character from the stream by extracting it.
     *
     * @tparam IntType The type to cast the character to.
     *
     * @return The read character.
     */
    template <typename IntType = int>
    IntType get();

    /**
     * Read characters from the stream until a newline or end-of-file is reached.
     *
     * @tparam CharType The character type of the resulting string.
     *
     * @param stream The string to insert extracted characters ito.
     *
     * @return True if any characters were read.
     */
    template <typename CharType>
    bool getline(std::basic_string<CharType> &result);

    /**
     * @return True if the stream has reached end-of-file.
     */
    bool eof();

protected:
    /**
     * Protected constructor.
     */
    UTF8Stream() = default;

    /**
     * Read the next character from the stream without extracting it.
     *
     * @return The peeked character.
     */
    virtual int peek_internal() = 0;

    /**
     * Read the next character from the stream by extracting it.
     *
     * @return The read character.
     */
    virtual int get_internal() = 0;

    /**
     * Check if a character represents end-of-file for the stream.
     *
     * @param ch The character to check.
     *
     * @return True if the character represents end-of-file.
     */
    virtual bool is_eof(int ch) = 0;
};

/**
 * UTF8Stream implementation for character type "char".
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 29, 2020
 */
class UTF8CharStream : public UTF8Stream
{
public:
    explicit UTF8CharStream(std::basic_istream<char> &stream);

protected:
    int peek_internal() override;
    int get_internal() override;
    bool is_eof(int ch) override;

private:
    std::basic_streambuf<char> *m_stream_buffer;
};

/**
 * UTF8Stream implementation for character type "char8_t".
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 29, 2020
 */
class UTF8Char8Stream : public UTF8Stream
{
public:
    explicit UTF8Char8Stream(std::basic_istream<char8_t> &stream);

protected:
    int peek_internal() override;
    int get_internal() override;
    bool is_eof(int ch) override;

private:
    std::basic_streambuf<char8_t> *m_stream_buffer;
};

//==================================================================================================
template <typename CharType>
std::unique_ptr<UTF8Stream> UTF8Stream::create(std::basic_istream<CharType> &stream)
{
    static_assert(supports_utf8_stream<CharType>());

    if constexpr (std::is_same_v<CharType, char>)
    {
        return std::make_unique<UTF8CharStream>(stream);
    }
    else if constexpr (std::is_same_v<CharType, char8_t>)
    {
        return std::make_unique<UTF8Char8Stream>(stream);
    }
}

//==================================================================================================
template <typename IntType>
inline IntType UTF8Stream::peek()
{
    return static_cast<IntType>(peek_internal());
}

//==================================================================================================
template <typename IntType>
inline IntType UTF8Stream::get()
{
    return static_cast<IntType>(get_internal());
}

//==================================================================================================
template <typename CharType>
bool UTF8Stream::getline(std::basic_string<CharType> &result)
{
    static constexpr const int s_new_line = 0x0a;

    result.clear();
    int ch;

    while (!eof() && ((ch = get_internal()) != s_new_line))
    {
        result += ch;
    }

    return !eof() || !result.empty();
}

//==================================================================================================
inline int UTF8CharStream::peek_internal()
{
    return static_cast<int>(m_stream_buffer->sgetc());
}

//==================================================================================================
inline int UTF8CharStream::get_internal()
{
    return static_cast<int>(m_stream_buffer->sbumpc());
}

//==================================================================================================
inline int UTF8Char8Stream::peek_internal()
{
    return static_cast<int>(m_stream_buffer->sgetc());
}

//==================================================================================================
inline int UTF8Char8Stream::get_internal()
{
    return static_cast<int>(m_stream_buffer->sbumpc());
}

} // namespace fly
