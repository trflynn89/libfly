# JSON Parser

Benchmark of the libfly JSON parser against the popular {fmt} library and the STL:

* [libfly](/fly/types/string/string.hpp)
* [{fmt}](https://github.com/fmtlib/fmt)

## Results

All results below are the median of 1000001 iterations of creating a formatted string.

### Formatting with floats

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         1.784 |
| STL IO Streams |         1.877 |
| {fmt}          |         0.586 |

### Formatting without floats

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         0.748 |
| {fmt}          |         0.411 |
| STL IO Streams |         1.011 |

## Profiles

### Formatting with floats

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 10.45      0.46     0.07  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<double, false>(fly::detail::BasicFormatSpecifier<char>&&, double const&)
 10.45      0.53     0.07  1000001     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)
  8.96      0.59     0.06  5000005     0.00     0.00  void fly::detail::BasicFormatParameters<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::visit<fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)::{lambda(auto:1&&, auto:2 const&)#1}, 1ul>(fly::detail::BasicFormatSpecifier<char>&&, fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)::{lambda(auto:1&&, auto:2 const&)#1}) const
  5.97      0.63     0.04 11000011     0.00     0.00  TLS init function for fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::s_stream
  2.99      0.65     0.02  1000013     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  1.49      0.66     0.01  2000004     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::~BasicStreamModifiers()
  1.49      0.67     0.01  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<unsigned char, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned char, bool)
  0.00      0.67     0.00  2000004     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::BasicStreamModifiers(std::ostream&)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<char [4], false>(fly::detail::BasicFormatSpecifier<char>&&, char const (&) [4])
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>(fly::detail::BasicFormatSpecifier<char>&&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<unsigned int, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned int, bool)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::append_string<char [4], false>(char const (&) [4], unsigned long)
  0.00      0.67     0.00  1000001     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned int>::type, signed char>, std::is_same<std::remove_cv<unsigned int>::type, short>, std::is_same<std::remove_cv<unsigned int>::type, int>, std::is_same<std::remove_cv<unsigned int>::type, long>, std::is_same<std::remove_cv<unsigned int>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned int>::type, unsigned char>, std::is_same<std::remove_cv<unsigned int>::type, unsigned short>, std::is_same<std::remove_cv<unsigned int>::type, unsigned int>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned int>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned int>(char*, char*, unsigned int, int)
  0.00      0.67     0.00     2131     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.67     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.67     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
```

### Formatting without floats

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 16.42      0.50     0.11  1000001     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>&&)
  7.46      0.62     0.05  3000003     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<unsigned int, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned int, bool)
  4.48      0.65     0.03  3000003     0.00     0.00  TLS init function for fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::s_stream
  1.49      0.67     0.01                             fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)
  0.00      0.67     0.00  3000003     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned int>::type, signed char>, std::is_same<std::remove_cv<unsigned int>::type, short>, std::is_same<std::remove_cv<unsigned int>::type, int>, std::is_same<std::remove_cv<unsigned int>::type, long>, std::is_same<std::remove_cv<unsigned int>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned int>::type, unsigned char>, std::is_same<std::remove_cv<unsigned int>::type, unsigned short>, std::is_same<std::remove_cv<unsigned int>::type, unsigned int>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned int>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned int>(char*, char*, unsigned int, int)
  0.00      0.67     0.00  1000013     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<char [4], false>(fly::detail::BasicFormatSpecifier<char>&&, char const (&) [4])
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>(fly::detail::BasicFormatSpecifier<char>&&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<unsigned char, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned char, bool)
  0.00      0.67     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::append_string<char [4], false>(char const (&) [4], unsigned long)
  0.00      0.67     0.00     2131     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.67     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.67     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.67     0.00      139     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.67     0.00      120     0.00     0.00  void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)
```
