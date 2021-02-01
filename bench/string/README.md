# JSON Parser

Benchmark of the libfly JSON parser against the popular {fmt} library and the STL:

* [libfly](/fly/types/string/string.hpp)
* [{fmt}](https://github.com/fmtlib/fmt)

## Results

All results below are the median of 1000001 iterations of creating a formatted string.

### Formatting with floats

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         1.626 |
| STL IO Streams |         1.877 |
| {fmt}          |         0.586 |

### Formatting without floats

| Formatter      | Duration (ns) |
| :--            |           --: |
| libfly         |         0.665 |
| {fmt}          |         0.411 |
| STL IO Streams |         1.011 |

## Profiles

### Formatting with floats

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 15.49      0.47     0.11  1000001     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>&&)
  5.63      0.59     0.04  2000002     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<double, false>(fly::detail::BasicFormatSpecifier<char>&&, double const&)
  5.63      0.63     0.04  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<unsigned long, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned long, bool)
  4.23      0.66     0.03  2000004     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::BasicStreamModifiers(std::ostream&)
  2.82      0.68     0.02  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<unsigned int, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned int, bool)
  1.41      0.70     0.01  1000013     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  1.41      0.71     0.01  1000001     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned int>::type, signed char>, std::is_same<std::remove_cv<unsigned int>::type, short>, std::is_same<std::remove_cv<unsigned int>::type, int>, std::is_same<std::remove_cv<unsigned int>::type, long>, std::is_same<std::remove_cv<unsigned int>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned int>::type, unsigned char>, std::is_same<std::remove_cv<unsigned int>::type, unsigned short>, std::is_same<std::remove_cv<unsigned int>::type, unsigned int>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned int>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned int>(char*, char*, unsigned int, int)
  0.00      0.71     0.00  2000004     0.00     0.00  fly::detail::BasicStreamModifiers<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::~BasicStreamModifiers()
  0.00      0.71     0.00  2000002     0.00     0.00  fly::benchmark::Table<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double>::~Table()
  0.00      0.71     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, int, double, char const (&) [4], decltype(nullptr), char>::format_value<unsigned char, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned char, bool)
  0.00      0.71     0.00  1000001     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned long>::type, signed char>, std::is_same<std::remove_cv<unsigned long>::type, short>, std::is_same<std::remove_cv<unsigned long>::type, int>, std::is_same<std::remove_cv<unsigned long>::type, long>, std::is_same<std::remove_cv<unsigned long>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned long>::type, unsigned char>, std::is_same<std::remove_cv<unsigned long>::type, unsigned short>, std::is_same<std::remove_cv<unsigned long>::type, unsigned int>, std::is_same<std::remove_cv<unsigned long>::type, unsigned long>, std::is_same<std::remove_cv<unsigned long>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned long>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned long>(char*, char*, unsigned long, int)
  0.00      0.71     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.71     0.00     1172     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.71     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
```

### Formatting without floats

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 15.38      0.40     0.10  3000003     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<unsigned int, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned int, bool)
 12.31      0.57     0.08  1000001     0.00     0.00  fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format(fly::detail::BasicFormatString<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>&&)
  3.08      0.59     0.02  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<char [4], false>(fly::detail::BasicFormatSpecifier<char>&&, char const (&) [4])
  3.08      0.61     0.02        1    20.00   650.00  void (anonymous namespace)::run_format_test<std::integral_constant<bool, false> >(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&&)
  1.54      0.62     0.01  3000003     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned int>::type, signed char>, std::is_same<std::remove_cv<unsigned int>::type, short>, std::is_same<std::remove_cv<unsigned int>::type, int>, std::is_same<std::remove_cv<unsigned int>::type, long>, std::is_same<std::remove_cv<unsigned int>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned int>::type, unsigned char>, std::is_same<std::remove_cv<unsigned int>::type, unsigned short>, std::is_same<std::remove_cv<unsigned int>::type, unsigned int>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long>, std::is_same<std::remove_cv<unsigned int>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned int>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned int>(char*, char*, unsigned int, int)
  1.54      0.64     0.01  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<unsigned long, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned long, bool)
  1.54      0.65     0.01  1000001     0.00     0.00  std::enable_if<std::__or_<std::__or_<std::is_same<std::remove_cv<unsigned long>::type, signed char>, std::is_same<std::remove_cv<unsigned long>::type, short>, std::is_same<std::remove_cv<unsigned long>::type, int>, std::is_same<std::remove_cv<unsigned long>::type, long>, std::is_same<std::remove_cv<unsigned long>::type, long long> >, std::__or_<std::is_same<std::remove_cv<unsigned long>::type, unsigned char>, std::is_same<std::remove_cv<unsigned long>::type, unsigned short>, std::is_same<std::remove_cv<unsigned long>::type, unsigned int>, std::is_same<std::remove_cv<unsigned long>::type, unsigned long>, std::is_same<std::remove_cv<unsigned long>::type, unsigned long long> >, std::is_same<char, std::remove_cv<unsigned long>::type> >::value, std::to_chars_result>::type std::__to_chars_i<unsigned long>(char*, char*, unsigned long, int)
  0.00      0.65     0.00  1000013     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::reserve(unsigned long)
  0.00      0.65     0.00  1000001     0.00     0.00  void fly::detail::BasicStringFormatter<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, int, int, char const (&) [4], decltype(nullptr), char>::format_value<unsigned char, false>(fly::detail::BasicFormatSpecifier<char>&&, unsigned char, bool)
  0.00      0.65     0.00     1196     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.65     0.00     1172     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.65     0.00     1016     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
```
