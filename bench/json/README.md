# JSON Parser

Benchmark of the libfly JSON parser against a couple of popular C++ JSON libraries:

* [libfly](/fly/parser/json_parser.hpp)
* [Boost.JSON](https://github.com/boostorg/json)
* [JSON for Modern C++](https://github.com/nlohmann/json)

## Results

All results below are the median of 11 iterations of the indicated JSON file. Obviously, libfly
could use some work :)

### [all_unicode.json](https://github.com/nlohmann/json_test_data/blob/master/json_nlohmann_tests/all_unicode.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |       141.364 |       52.074 |
| boost    |        30.795 |      239.044 |
| nlohmann |        80.322 |       91.648 |


### [canada.json](https://github.com/miloyip/nativejson-benchmark/blob/master/data/canada.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |        36.768 |       58.386 |
| boost    |         4.725 |      454.373 |
| nlohmann |        22.047 |       97.373 |


## Profile

A profile of parsing the all_unicode.json file indicates most time is spent in the `fly::Json` move
constructor.

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 37.32      0.53     0.53 35301398     0.00     0.00  fly::Json::Json(fly::Json&&)
 11.97      0.70     0.17 12232704     0.00     0.00  fly::parser::JsonParser::parse_quoted_string[abi:cxx11]()
  9.86      0.84     0.14 70602796     0.00     0.00  std::__detail::__variant::_Variant_storage<false, decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>::_M_reset()
  7.04      0.94     0.10 12234321     0.00     0.00  unsigned int fly::detail::BasicUnicode<char>::codepoint_from_string<char const*>(char const*&, char const* const&)
  5.63      1.02     0.08 36698134     0.00     0.00  fly::parser::JsonParser::consume_whitespace_and_comments()
  5.63      1.10     0.08 12232704     0.00     0.00  fly::Json::validate_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&&)
  4.93      1.17     0.07 61163531     0.00     0.00  fly::parser::JsonParser::consume_whitespace()
  3.52      1.22     0.05 24465397     0.00     0.00  fly::parser::JsonParser::consume_token(fly::parser::JsonParser::Token)
  3.52      1.27     0.05 12232693     0.00     0.00  fly::parser::JsonParser::consume_comma(fly::parser::JsonParser::Token)
  2.11      1.30     0.03 12232715     0.00     0.00  fly::parser::JsonParser::parse_json()
  2.11      1.33     0.03 12232704     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > fly::Json::convert_to_string<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
  2.11      1.36     0.03 12232704     0.00     0.00  fly::Json::push_back(fly::Json&&)
  1.41      1.38     0.02 24465408     0.00     0.00  fly::parser::JsonParser::state_for_object_or_array(fly::parser::JsonParser::Token)
  0.70      1.41     0.01 61163542     0.00     0.00  fly::parser::JsonParser::is_whitespace(fly::parser::JsonParser::Token) const
  0.70      1.42     0.01       11     0.00     0.08  fly::parser::JsonParser::parse_array()
  0.00      1.42     0.00      385     0.00     0.00  fly::Json::read_escaped_character(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&, __gnu_cxx::__normal_iterator<char*, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >&)
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
