# JSON Parser

Benchmark of the libfly JSON parser against the popular {fmt} library and the STL:

* [libfly](/fly/types/string/string.hpp)
* [{fmt}](https://github.com/fmtlib/fmt)

## Results

Results below are the median of 1000001 iterations of creating a formatted string. Obviously, libfly
could use some work :)

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         4.496 |
| {fmt}          |         0.583 |
| STL IO Streams |         1.821 |

## Profile

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 16.09      0.67     0.14  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_internal<double, int, double, char const (&) [4], decltype(nullptr), char>(std::ostream&, fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<double>::type, std::type_identity<int>::type, std::type_identity<double>::type, std::type_identity<char const (&) [4]>::type, std::type_identity<decltype(nullptr)>::type, std::type_identity<char>::type>&&, fly::detail::BasicFormatParameters<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<double>::type, std::type_identity<int>::type, std::type_identity<double>::type, std::type_identity<char const (&) [4]>::type, std::type_identity<decltype(nullptr)>::type, std::type_identity<char>::type>&&)
  3.45      0.74     0.03  6000011     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::~BasicStreamModifiers()
  3.45      0.77     0.03  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<double>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, double const&)
  1.15      0.78     0.01  6000007     0.00     0.00  std::noshowpoint(std::ios_base&)
  1.15      0.79     0.01  6000007     0.00     0.00  std::nouppercase(std::ios_base&)
  1.15      0.80     0.01  2000002     0.00     0.00  fly::BinaryDecoder::decode_internal(std::istream&, std::ostream&)
  1.15      0.81     0.01  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_internal<unsigned int, char&, char&>(std::ostream&, fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<unsigned int>::type, std::type_identity<char&>::type, std::type_identity<char&>::type>&&, fly::detail::BasicFormatParameters<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<unsigned int>::type, std::type_identity<char&>::type, std::type_identity<char&>::type>&&)
  1.15      0.82     0.01  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<char>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, char const&)
  1.15      0.83     0.01  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<int>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, int const&)
  1.15      0.84     0.01        2     5.00     5.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  0.57      0.87     0.01  3000004     0.00     0.00  std::left(std::ios_base&)
  0.57      0.87     0.01  3000003     0.00     0.00  std::right(std::ios_base&)
  0.00      0.87     0.00  6000007     0.00     0.00  std::noshowbase(std::ios_base&)
  0.00      0.87     0.00  5000005     0.00     0.00  std::noboolalpha(std::ios_base&)
  0.00      0.87     0.00  1000019     0.00     0.00  fly::operator<<(std::ostream&, fly::Styler const&)
  0.00      0.87     0.00  1000002     0.00     0.00  std::boolalpha(std::ios_base&)
  0.00      0.87     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<char [4]>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, char const (&) [4])
  0.00      0.87     0.00     2128     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.87     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.87     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.87     0.00      138     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.87     0.00      120     0.00     0.00  void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)
```
