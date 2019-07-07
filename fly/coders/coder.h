#pragma once

#include <filesystem>
#include <istream>
#include <ostream>
#include <string>

namespace fly {

class BitStreamReader;
class BitStreamWriter;

/**
 * Virtual interface to encode and decode a file or string. Coders for specific
 * algorithms should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class Coder
{
public:
    /**
     * Destructor.
     */
    virtual ~Coder() = default;

    /**
     * Encode a string.
     *
     * @param string String holding the contents to encode.
     * @param string String to store the encoded contents.
     *
     * @return bool True if the input string was successfully encoded.
     */
    bool EncodeString(const std::string &, std::string &) noexcept;

    /**
     * Encode a file.
     *
     * @param path Path holding the contents to encode.
     * @param path Path to store the encoded contents.
     *
     * @return bool True if the input file was successfully encoded.
     */
    bool EncodeFile(
        const std::filesystem::path &,
        const std::filesystem::path &) noexcept;

    /**
     * Decode a string.
     *
     * @param string String holding the contents to decode.
     * @param string String to store the decoded contents.
     *
     * @return bool True if the input string was successfully decoded.
     */
    bool DecodeString(const std::string &, std::string &) noexcept;

    /**
     * Decode a file.
     *
     * @param path Path holding the contents to decode.
     * @param path Path to store the decoded contents.
     *
     * @return bool True if the input file was successfully decoded.
     */
    bool DecodeFile(
        const std::filesystem::path &,
        const std::filesystem::path &) noexcept;

protected:
    /**
     * Encode a stream.
     *
     * @param istream Stream holding the contents to encode.
     * @param BitStreamWriter Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    virtual bool EncodeInternal(std::istream &, BitStreamWriter &) noexcept = 0;

    /**
     * Decode a stream.
     *
     * @param istream Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    virtual bool DecodeInternal(BitStreamReader &, std::ostream &) noexcept = 0;
};

} // namespace fly
