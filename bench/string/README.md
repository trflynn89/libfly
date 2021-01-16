# JSON Parser

Benchmark of the libfly JSON parser against the popular {fmt} library and the STL:

* [libfly](/fly/types/string/string.hpp)
* [{fmt}](https://github.com/fmtlib/fmt)

## Results

Results below are the median of 1000001 iterations of creating a formatted string. Obviously, libfly
could use some work :)

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         3.711 |
| {fmt}          |         0.595 |
| STL IO Streams |         1.833 |

## Profile

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 35.90      0.14     0.14  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_internal<double, int, double, char const (&) [4], decltype(nullptr), char>(std::ostream&, fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<double>::type, std::type_identity<int>::type, std::type_identity<double>::type, std::type_identity<char const (&) [4]>::type, std::type_identity<decltype(nullptr)>::type, std::type_identity<char>::type>&&, fly::detail::BasicFormatParameters<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::type_identity<double>::type, std::type_identity<int>::type, std::type_identity<double>::type, std::type_identity<char const (&) [4]>::type, std::type_identity<decltype(nullptr)>::type, std::type_identity<char>::type>&&)
 10.26      0.25     0.04  6000011     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::~BasicStreamModifiers()
 10.26      0.29     0.04  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<char [4]>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, char const (&) [4])
  5.13      0.34     0.02  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<double>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, double const&)
  5.13      0.36     0.02  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<char>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, char const&)
  2.56      0.38     0.01        1    10.00    10.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_replace(unsigned long, unsigned long, char const*, unsigned long)
  2.56      0.39     0.01                             std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::swap(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&)
  0.00      0.39     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::format_value<int>(std::ostream&, fly::detail::BasicFormatSpecifier<char>&&, int const&)
  0.00      0.39     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.39     0.00     1172     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.39     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.39     0.00      138     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.39     0.00      120     0.00     0.00  void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)
```
