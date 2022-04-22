#pragma once

#include <cstddef>
#include <exception>
#include <string>

namespace fly {

class Json;

/**
 * Generic exception to be raised if an error was encountered creating, accessing, or modifying a
 * Json instance.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 24, 2017
 */
class JsonException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param message Message indicating what error was encountered.
     */
    explicit JsonException(std::string &&message) noexcept;

    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     * @param message Message indicating what error was encountered.
     */
    JsonException(Json const &json, std::string &&message) noexcept;

    /**
     * @return C-string representing this exception.
     */
    virtual char const *what() const noexcept override;

protected:
    /**
     * Constructor for subclasses.
     *
     * @param class_name Name of the subclass.
     * @param message Message indicating what error was encountered.
     */
    JsonException(char const *class_name, std::string &&message) noexcept;

private:
    std::string const m_message;
};

/**
 * Exception to be raised for generic JsonIterator errors.
 */
class JsonIteratorException : public JsonException
{
public:
    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     * @param message Message indicating what error was encountered.
     */
    JsonIteratorException(Json const &json, std::string &&message) noexcept;
};

/**
 * Exception to be raised for illegally comparing iterators of two different Json instances.
 */
class BadJsonComparisonException : public JsonException
{
public:
    /**
     * Constructor.
     *
     * @param json1 The first Json instance in the comparison.
     * @param json2 The second Json instance in the comparison.
     */
    BadJsonComparisonException(Json const &json1, Json const &json2) noexcept;
};

/**
 * Exception to be raised trying to dereference an empty or past-the-end JsonIterator instance.
 */
class NullJsonException : public JsonException
{
public:
    /**
     * Constructor.
     */
    NullJsonException() noexcept;

    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     */
    explicit NullJsonException(Json const &json) noexcept;
};

/**
 * Exception to be raised trying to create an iterator that escapes the range [begin, end] of a Json
 * instance.
 */
class OutOfRangeJsonException : public JsonException
{
public:
    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     * @param offset The iterator offset attempted to be accessed.
     */
    OutOfRangeJsonException(Json const &json, std::ptrdiff_t offset) noexcept;

    /**
     * @return The iterator offset attempted to be accessed.
     */
    std::ptrdiff_t offset() const;

private:
    std::ptrdiff_t m_offset;
};

} // namespace fly
