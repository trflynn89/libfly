# JSON Parser

Benchmark of the libfly JSON parser against a couple of popular C++ JSON libraries:

* [libfly](/fly/parser/json_parser.hpp)
* [Boost.JSON](https://github.com/boostorg/json)
* [JSON for Modern C++](https://github.com/nlohmann/json)

## Results

All results below are the median of 11 iterations of the indicated JSON file. Obviously, libfly
could use some work :)

### [all_unicode.json](/test/parser/json/unicode/all_unicode.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |       286.788 |       25.668 |
| boost    |        72.839 |      101.063 |
| nlohmann |       199.067 |       36.979 |

### [canada.json](/bench/json/data/canada.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |       105.899 |       20.272 |
| boost    |        12.037 |      178.353 |
| nlohmann |        54.929 |       39.082 |

### [gsoc-2018.json](/bench/json/data/gsoc-2018.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |        63.754 |       49.780 |
| boost    |        14.083 |      225.350 |
| nlohmann |        32.914 |       96.423 |


## Profile

A profile of parsing the all_unicode.json file indicates most time is spent in the `fly::Json` move
constructor.

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 46.43      0.13     0.13  3209219     0.00     0.00  fly::Json::Json(fly::Json&&)
 10.71      0.16     0.03  1112064     0.00     0.00  fly::JsonParser::parse_quoted_string[abi:cxx11]()
  7.14      0.18     0.02  4321284     0.00     0.00  fly::Json::~Json()
  7.14      0.20     0.02  1112064     0.00     0.00  fly::Json::push_back(fly::Json&&)
  7.14      0.22     0.02  1112064     0.00     0.00  fly::Json::Json<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&&)
  3.57      0.23     0.01  4382781     0.00     0.00  std::_Function_handler<unsigned int (), fly::detail::BasicStringUnicode<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::decode_codepoint<__gnu_cxx::__normal_iterator<char const*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >(__gnu_cxx::__normal_iterator<char const*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >&, __gnu_cxx::__normal_iterator<char const*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > const&)::{lambda()#1}>::_M_invoke(std::_Any_data const&)
  3.57      0.24     0.01  3336194     0.00     0.00  fly::JsonParser::consume_whitespace_and_comments()
  3.57      0.25     0.01  2224127     0.00     0.00  fly::JsonParser::consume_token(fly::JsonParser::Token)
  3.57      0.26     0.01  1112065     0.00     0.00  fly::Json::is_object() const
  3.57      0.27     0.01  1112064     0.00     0.00  fly::Json::validate_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&&)
  3.57      0.28     0.01        1    10.00   201.67  fly::JsonParser::parse_array()
  0.00      0.28     0.00  5560322     0.00     0.00  fly::JsonParser::is_whitespace(fly::JsonParser::Token) const
  0.00      0.28     0.00  5560321     0.00     0.00  fly::JsonParser::consume_whitespace()
```

The move constructor is about as simple as it gets:

```c++
Json::Json(Json &&json) noexcept : m_value(std::move(json.m_value))
{
    json.m_value = nullptr;
}
```

Maybe there is a more efficient way to move `std::variant`? I did not see much of an improvement by
swapping the `std::variant` with a plain union. Alternatively, maybe the parser could hold off on
actually creating `fly::Json` objects as long as possible.
