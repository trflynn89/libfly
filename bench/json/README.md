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
| libfly   |       236.902 |       31.073 |
| boost    |        67.010 |      109.854 |
| nlohmann |       197.665 |       37.242 |

### [canada.json](/bench/json/data/canada.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |        88.977 |       24.127 |
| boost    |        11.938 |      179.822 |
| nlohmann |        54.969 |       39.054 |

### [gsoc-2018.json](/bench/json/data/gsoc-2018.json)

| Parser   | Duration (ms) | Speed (MB/s) |
| :--      |           --: |          --: |
| libfly   |        41.520 |       76.437 |
| boost    |        14.111 |      224.903 |
| nlohmann |        32.927 |       96.386 |

## Profile

A profile of parsing the all_unicode.json file indicates most time is spent in the `fly::Json` move
constructor.

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 27.59      0.08     0.08  3209218     0.00     0.00  fly::Json::Json(fly::Json&&)
 13.79      0.12     0.04  1112064     0.00     0.00  fly::JsonParser::parse_quoted_string[abi:cxx11]()
 13.79      0.16     0.04  1112064     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > fly::Json::convert_to_string<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
 10.34      0.19     0.03  4321279     0.00     0.00  std::__detail::__variant::__gen_vtable_impl<std::__detail::__variant::_Multi_array<void (*)(std::__detail::__variant::_Variant_storage<false, decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>::_M_reset()::{lambda(auto:1&&)#1}&&, std::variant<decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>&)>, std::integer_sequence<unsigned long, 1ul> >::__visit_invoke(std::__detail::__variant::_Variant_storage<false, decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>::_M_reset()::{lambda(auto:1&&)#1}, std::variant<decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>)
  6.90      0.21     0.02  5560321     0.00     0.00  fly::JsonParser::consume_whitespace()
  6.90      0.23     0.02  1112092     0.00     0.00  fly::Json::push_back(fly::Json&&)
  6.90      0.25     0.02  1112064     0.00     0.00  fly::Json::validate_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&&)
  3.45      0.26     0.01  4321284     0.00     0.00  std::__detail::__variant::__gen_vtable_impl<std::__detail::__variant::_Multi_array<void (*)(std::__detail::__variant::_Variant_storage<false, decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>::_M_reset()::{lambda(auto:1&&)#1}&&, std::variant<decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>&)>, std::integer_sequence<unsigned long, 0ul> >::__visit_invoke(std::__detail::__variant::_Variant_storage<false, decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>::_M_reset()::{lambda(auto:1&&)#1}, std::variant<decltype(nullptr), std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, fly::Json, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, fly::Json> > >, std::vector<fly::Json, std::allocator<fly::Json> >, bool, long, unsigned long, long double>)
  3.45      0.27     0.01  3336194     0.00     0.00  fly::JsonParser::consume_whitespace_and_comments()
  3.45      0.28     0.01  1112065     0.00     0.00  fly::JsonParser::parse_json()
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
