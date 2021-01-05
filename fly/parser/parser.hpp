#pragma once

#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/numeric/endian.hpp"
#include "fly/types/string/string.hpp"

#include <bit>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

namespace fly {

/**
 * Virtual interface to parse a file or string. Parsers for specific formats should inherit from
 * this class.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
class Parser
{
public:
    /**
     * Destructor.
     */
    virtual ~Parser() = default;

    /**
     * Parse a string and retrieve the parsed values.
     *
     * The encoding of the string is inferred from the templated string type:
     *
     *     1. std::string - UTF-8
     *     2. std::wstring - UTF-16 on Windows, UTF-32 on Linux and macOS
     *     3. std::u8string - UTF-8
     *     4. std::u16string - UTF-16
     *     5. std::u32string - UTF-32
     *
     * Any string which is not encoded with UTF-8 will first be converted to a UTF-8 encoded string
     * before being passed to concrete parsers.
     *
     * @tparam StringType The type of string to parse.
     *
     * @param contents String contents to parse.
     *
     * @return If successful, the parsed values. Otherwise, an uninitialized value.
     */
    template <typename StringType>
    std::optional<Json> parse_string(const StringType &contents);

    /**
     * Parse a file and retrieve the parsed values.
     *
     * The encoding of the file is inferred from the presence of a byte order mark. If a BOM is not
     * present, the parser will assume UTF-8 encoding. If a BOM is present, the following BOM
     * representations are supported:
     *
     *     1. UTF-8 (0xef 0xbb 0xbf)
     *     2. UTF-16 big endian (0xfe 0xff)
     *     3. UTF-16 little endian (0xff 0xfe)
     *     4. UTF-32 big endian (0x00 0x00 0xfe 0xff)
     *     5. UTF-32 little endian (0xff 0xfe 0x00 0x00)
     *
     * Any file which is not encoded with UTF-8 will first be converted to a UTF-8 encoded string
     * before being passed to concrete parsers.
     *
     * @param path Path to the file to parse.
     *
     * @return If successful, the parsed values. Otherwise, an uninitialized value.
     */
    std::optional<Json> parse_file(const std::filesystem::path &path);

protected:
    /**
     * Parse a UTF-8 encoded stream and retrieve the parsed values. Concrete parsers may read data
     * from the underlyins string through the peek(), get(), discard(), and eof() methods.
     *
     * @return If successful, the parsed values. Otherwise, an uninitialized value.
     */
    virtual std::optional<Json> parse_internal() = 0;

    /**
     * Read the next symbol from the stream without extracting it.
     *
     * @tparam SymbolType The type to cast the symbol to.
     *
     * @return The peeked symbol.
     */
    template <typename SymbolType = std::ios::int_type>
    SymbolType peek();

    /**
     * Read the next symbol from the stream by extracting it.
     *
     * @tparam SymbolType The type to cast the symbol to.
     *
     * @return The read symbol.
     */
    template <typename SymbolType = std::ios::int_type>
    SymbolType get();

    /**
     * Discard the next symbol from the stream by extracting it.
     */
    void discard();

    /**
     * @return True if the stream has reached end-of-file.
     */
    bool eof();

    /**
     * @return The current line number in the stream.
     */
    std::uint32_t line() const;

    /**
     * @return The current column number in the stream.
     */
    std::uint32_t column() const;

private:
    /**
     * Enumeration of supported encodings as indicated by a byte order mark.
     */
    enum class Encoding : std::uint8_t
    {
        UTF8,
        UTF16BigEndian,
        UTF16LittleEndian,
        UTF32BigEndian,
        UTF32LittleEndian,
    };

    /**
     * Parse a stream and retrieve the parsed values. Check for the presence of a byte order mark,
     * and convert the stream to a UTF-8 encoded string before parsing if needed.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed values. Otherwise, an uninitialized value.
     */
    std::optional<Json> parse_stream(std::istream &&stream);

    /**
     * Parse a non-UTF-8 encoded stream and convert the result to a UTF-8 encoded string.
     *
     * @tparam StringType String type which can hold the non-UTF-8 encoded contents.
     * @tparam Endianness The endianness of the non-UTF-8 encoded contents.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, a copy of the stream with UTF-8 encoding. Otherwise, an uninitialized
     *         value.
     */
    template <typename StringType, std::endian Endianness>
    std::optional<std::string> ensure_utf8(std::istream &stream) const;

    /**
     * If present, parse the leading byte order mark to determine the stream's Unicode encoding. If
     * no BOM is present, default to UTF-8.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The detected Unicode encoding.
     */
    Encoding parse_byte_order_mark(std::istream &stream) const;

    std::streambuf *m_stream_buffer {nullptr};

    std::uint32_t m_line {0};
    std::uint32_t m_column {0};
};

//==================================================================================================
template <typename StringType>
std::optional<Json> Parser::parse_string(const StringType &contents)
{
    using CharType = typename StringType::value_type;

    if constexpr (std::is_same_v<CharType, std::ios::char_type>)
    {
        std::basic_istringstream<CharType> stream(contents);
        return parse_stream(std::move(stream));
    }
    else
    {
        auto utf8_contents = BasicString<StringType>::template convert<std::string>(contents);
        return utf8_contents ? parse_string(*utf8_contents) : std::nullopt;
    }
}

//==================================================================================================
template <typename StringType, std::endian Endianness>
std::optional<std::string> Parser::ensure_utf8(std::istream &stream) const
{
    using CharType = typename StringType::value_type;

    static constexpr const std::uint8_t s_char_size = sizeof(CharType);
    StringType contents;

    while (stream)
    {
        CharType character = 0;

        for (std::uint8_t i = 0; stream && (i < s_char_size); ++i)
        {
            const std::uint8_t shift = 8 * (s_char_size - i - 1);
            character |= static_cast<CharType>(stream.get() & 0xff) << shift;
        }

        if (stream)
        {
            if constexpr (Endianness == std::endian::little)
            {
                character = endian_swap(character);
            }

            contents.push_back(character);
        }
    }

    return BasicString<StringType>::template convert<std::string>(contents);
}

//==================================================================================================
template <typename SymbolType>
inline SymbolType Parser::peek()
{
    return static_cast<SymbolType>(m_stream_buffer->sgetc());
}

//==================================================================================================
template <typename SymbolType>
inline SymbolType Parser::get()
{
    static constexpr const std::ios::int_type s_new_line = 0x0a;

    const std::ios::int_type symbol = m_stream_buffer->sbumpc();

    if (symbol == s_new_line)
    {
        m_column = 1;
        ++m_line;
    }
    else
    {
        ++m_column;
    }

    return static_cast<SymbolType>(symbol);
}

//==================================================================================================
inline void Parser::discard()
{
    FLY_UNUSED(get());
}

//==================================================================================================
inline bool Parser::eof()
{
    return peek() == std::char_traits<std::ios::char_type>::eof();
}

} // namespace fly
