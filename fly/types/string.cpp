#include "fly/types/string.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <random>

namespace fly {

namespace {

    const std::string s_alphaNum =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

} // namespace

//==============================================================================
std::vector<std::string> String::Split(const std::string &input, char delim)
{
    return Split(input, delim, 0);
}

//==============================================================================
std::vector<std::string>
String::Split(const std::string &input, char delim, size_t max)
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
    auto is_non_space = [](int ch) { return !std::isspace(ch); };

    // Remove leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), is_non_space));

    // Remove trailing whitespace
    str.erase(
        std::find_if(str.rbegin(), str.rend(), is_non_space).base(), str.end());
}

//==============================================================================
void String::ReplaceAll(
    std::string &target,
    const std::string &search,
    const char &replace)
{
    size_t pos = target.find(search);

    while (!search.empty() && (pos != std::string::npos))
    {
        target.replace(pos, search.length(), 1, replace);
        pos = target.find(search);
    }
}

//==============================================================================
void String::ReplaceAll(
    std::string &target,
    const std::string &search,
    const std::string &replace)
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
    ReplaceAll(target, search, std::string());
}

//==============================================================================
std::string String::GenerateRandomString(const size_t len)
{
    typedef std::uniform_int_distribution<short> short_distribution;

    auto now = std::chrono::system_clock::now().time_since_epoch();

    auto seed = static_cast<std::mt19937::result_type>(now.count());
    auto limit =
        static_cast<short_distribution::result_type>(s_alphaNum.size() - 1);

    std::mt19937 engine(seed);
    short_distribution distribution(0, limit);

    std::string ret;
    ret.reserve(len);

    for (size_t i = 0; i < len; ++i)
    {
        ret += s_alphaNum[distribution(engine)];
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

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("bool");
    }
    else if ((result < min) || (result > max))
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

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("char");
    }
    else if ((result < min) || (result > max))
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

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("uchar");
    }
    else if ((result < min) || (result > max))
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

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("short");
    }
    else if ((result < min) || (result > max))
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

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("ushort");
    }
    else if ((result < min) || (result > max))
    {
        throw std::out_of_range("ushort");
    }

    return static_cast<unsigned short>(result);
}

//==============================================================================
template <>
int String::Convert(const std::string &value)
{
    std::size_t index = 0;
    int result = std::stoi(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("int");
    }

    return result;
}

//==============================================================================
template <>
unsigned int String::Convert(const std::string &value)
{
    static const long long min = std::numeric_limits<unsigned int>::min();
    static const long long max = std::numeric_limits<unsigned int>::max();

    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("uint");
    }
    else if ((result < min) || (result > max))
    {
        throw std::out_of_range("uint");
    }

    return static_cast<unsigned int>(result);
}

//==============================================================================
template <>
long String::Convert(const std::string &value)
{
    std::size_t index = 0;
    long result = std::stol(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("long");
    }

    return result;
}

//==============================================================================
template <>
unsigned long String::Convert(const std::string &value)
{
    std::size_t index = 0;
    unsigned long result = std::stoul(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("ulong");
    }

    return result;
}

//==============================================================================
template <>
long long String::Convert(const std::string &value)
{
    std::size_t index = 0;
    long long result = std::stoll(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("llong");
    }

    return result;
}

//==============================================================================
template <>
unsigned long long String::Convert(const std::string &value)
{
    std::size_t index = 0;
    unsigned long long result = std::stoull(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("ullong");
    }

    return result;
}

//==============================================================================
template <>
float String::Convert(const std::string &value)
{
    std::size_t index = 0;
    float result = std::stof(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("float");
    }

    return result;
}

//==============================================================================
template <>
double String::Convert(const std::string &value)
{
    std::size_t index = 0;
    double result = std::stod(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("double");
    }

    return result;
}

//==============================================================================
template <>
long double String::Convert(const std::string &value)
{
    std::size_t index = 0;
    long double result = std::stold(value, &index);

    if (index != value.length())
    {
        throw std::invalid_argument("ldouble");
    }

    return result;
}

//==============================================================================
void String::format(std::ostream &stream, const char *fmt)
{
    stream << fmt;
}

} // namespace fly
