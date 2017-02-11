#include "string.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <cstdlib>
#include <sstream>

namespace fly {

//==============================================================================
const std::string String::s_alphaNum =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

const unsigned int String::s_asciiSize = 256;

UniformIntegerDevice<size_t, std::mt19937> String::s_randomDevice(0, String::s_alphaNum.size() - 1);

//==============================================================================
std::vector<std::string> String::Split(const std::string &input, char delim)
{
    return Split(input, delim, 0);
}

//==============================================================================
std::vector<std::string> String::Split(const std::string &input, char delim, size_t max)
{
    std::string item;
    std::stringstream ss(input);
    std::vector<std::string> elems;
    size_t numItems = 0;

    while (std::getline(ss, item, delim))
    {
        if (!item.empty())
        {
            if ((max > 0) && (++numItems > max))
            {
                elems.back() += delim;
                elems.back() += item;
            }
            else
            {
                elems.push_back(item);
            }
        }
    }

    return elems;
}

//==============================================================================
void String::Trim(std::string &str)
{
    // Remove leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        std::not1(std::ptr_fun<int, int>(std::isspace))));

    // Remove trailing whitespace
    str.erase(std::find_if(str.rbegin(), str.rend(),
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
}

//==============================================================================
void String::ReplaceAll(std::string &target, const std::string &search, const std::string &replace)
{
    size_t pos = target.find(search);

    while (!search.empty() && (pos != std::string::npos))
    {
        target.replace(pos, search.length(), replace);
        pos = target.find(search);
    }
}

//==============================================================================
void String::RemoveAll(std::string &target, const std::string &search)
{
    static const std::string empty;
    ReplaceAll(target, search, empty);
}

//==============================================================================
std::string String::GenerateRandomString(const unsigned int len)
{
    std::string ret;
    ret.reserve(len);

    for (unsigned int i = 0; i < len; ++i)
    {
        ret += s_alphaNum[s_randomDevice()];
    }

    return ret;
}

//==============================================================================
bool String::StartsWith(const std::string &source, const char &search)
{
    bool ret = false;

    if (!source.empty())
    {
        ret = (source[0] == search);
    }

    return ret;
}

//==============================================================================
bool String::StartsWith(const std::string &source, const std::string &search)
{
    bool ret = false;

    const size_t sourceSz = source.length();
    const size_t searchSz = search.length();

    if (sourceSz >= searchSz)
    {
        ret = (source.compare(0, searchSz, search) == 0);
    }

    return ret;
}

//==============================================================================
bool String::EndsWith(const std::string &source, const char &search)
{
    bool ret = false;

    const size_t sourceSz = source.length();

    if (sourceSz > 0)
    {
        ret = (source[sourceSz - 1] == search);
    }

    return ret;
}

//==============================================================================
bool String::EndsWith(const std::string &source, const std::string &search)
{
    bool ret = false;

    const size_t sourceSz = source.length();
    const size_t searchSz = search.length();

    if (sourceSz >= searchSz)
    {
        ret = (source.compare(sourceSz - searchSz, searchSz, search) == 0);
    }

    return ret;
}

//==============================================================================
bool String::WildcardMatch(const std::string &source, const std::string &search)
{
    static const char wildcard = '*';
    bool ret = !search.empty();

    const std::vector<std::string> segments = Split(search, wildcard);
    std::string::size_type pos = 0;

    if (!segments.empty())
    {
        if (ret && (search.front() != wildcard))
        {
            ret = StartsWith(source, segments.front());
        }
        if (ret && (search.back() != wildcard))
        {
            ret = EndsWith(source, segments.back());
        }

        for (auto it = segments.begin(); ret && (it != segments.end()); ++it)
        {
            pos = source.find(*it, pos);

            if (pos == std::string::npos)
            {
                ret = false;
            }
        }
    }

    return ret;
}

//==============================================================================
float String::CalculateEntropy(const std::string &source)
{
    long charCount[s_asciiSize] = { 0 };

    // Count the number of occurences of each ASCII character in the string
    for (auto it = source.begin(); it != source.end(); ++it)
    {
        unsigned int ascii = static_cast<unsigned int>(*it);

        if (ascii < s_asciiSize)
        {
            ++charCount[ascii];
        }
    }

    float entropy = 0.0;
    float length = static_cast<float>(source.length());

    // Calculate the entropy
    for (unsigned int i = 0; i < s_asciiSize; ++i)
    {
        long count = charCount[i];

        if (count > 0)
        {
            float pct = static_cast<float>(count) / length;
            entropy -= (pct * log2(pct));
        }
    }

    return entropy;
}

//==============================================================================
template <>
std::string String::Convert(const std::string &value)
{
    return value;
}

//==============================================================================
template <>
bool String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<bool>::min();
    static const long long max = std::numeric_limits<bool>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("bool");
    }

    return (result != 0);
}

//==============================================================================
template <>
char String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<char>::min();
    static const long long max = std::numeric_limits<char>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("char");
    }

    return static_cast<char>(result);
}

//==============================================================================
template <>
unsigned char String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<unsigned char>::min();
    static const long long max = std::numeric_limits<unsigned char>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("uchar");
    }

    return static_cast<unsigned char>(result);
}

//==============================================================================
template <>
short String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<short>::min();
    static const long long max = std::numeric_limits<short>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("short");
    }

    return static_cast<short>(result);
}

//==============================================================================
template <>
unsigned short String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<unsigned short>::min();
    static const long long max = std::numeric_limits<unsigned short>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("ushort");
    }

    return static_cast<unsigned short>(result);
}

//==============================================================================
template <>
int String::Convert(const std::string &value)
{
    return std::stoi(value);
}

//==============================================================================
template <>
unsigned int String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<unsigned int>::min();
    static const long long max = std::numeric_limits<unsigned int>::max();
    long long result = std::stoll(value);

    if ((result < min) || (result > max))
    {
        throw std::out_of_range("uint");
    }

    return static_cast<unsigned int>(result);
}

//==============================================================================
template <>
long String::Convert(const std::string &value)
{
    return std::stol(value);
}

//==============================================================================
template <>
unsigned long String::Convert(const std::string &value)
{
    return std::stoul(value);
}

//==============================================================================
template <>
long long String::Convert(const std::string &value)
{
    return std::stoll(value);
}

//==============================================================================
template <>
unsigned long long String::Convert(const std::string &value)
{
    return std::stoull(value);
}

//==============================================================================
template <>
float String::Convert(const std::string &value)
{
    return std::stof(value);
}

//==============================================================================
template <>
double String::Convert(const std::string &value)
{
    return std::stod(value);
}

//==============================================================================
template <>
long double String::Convert(const std::string &value)
{
    return std::stold(value);
}

//==============================================================================
void String::format(std::ostream &stream, const char *fmt)
{
    stream << fmt;
}

}
