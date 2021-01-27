# JSON Parser

Benchmark of the libfly JSON parser against the popular {fmt} library and the STL:

* [libfly](/fly/types/string/string.hpp)
* [{fmt}](https://github.com/fmtlib/fmt)

## Results

Results below are the median of 1000001 iterations of creating a formatted string. Obviously, libfly
could use some work :)

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         2.337 |
| {fmt}          |         0.592 |
| STL IO Streams |         1.857 |

## Profile

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 26.92      0.56     0.21  1000001     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)
  3.85      0.65     0.03  6000038     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::BasicStreamModifiers(std::ostream&)
  3.85      0.68     0.03  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::set_numeric_options<double>(fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >&, fly::detail::BasicFormatSpecifier<char> const&, double const&) const
  2.56      0.73     0.02  6000038     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::~BasicStreamModifiers()
  2.56      0.75     0.02  6000006     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::set_generic_options(fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >&, fly::detail::BasicFormatSpecifier<char> const&) const
  1.28      0.76     0.01        2     5.00     5.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  1.28      0.77     0.01                             std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_replace(unsigned long, unsigned long, char const*, unsigned long)
  1.28      0.78     0.01                             std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::swap(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&)
  0.00      0.78     0.00  1000001     0.00     0.00  void fly::detail::BasicStringStreamer<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::stream_string<char const (&) [4]>(std::ostream&, char const (&) [4], unsigned long)
  0.00      0.78     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.78     0.00     1172     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.78     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
```
