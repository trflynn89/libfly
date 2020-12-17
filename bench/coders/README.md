# Coders

Benchmark of the libfly coders with [enwik8](http://mattmahoney.net/dc/enwik8.zip), an approximately
100 MB file. This file is not comitted into git, so it must be fetched, unzipped, and stored in the
[data](/bench/coders/data) folder.

## Results

All results below are the median of 11 iterations of encoding and decoding the enwik8 file.

### [Huffman Coder](/fly/coders/huffman)

| Direction | Duration (ms) | Speed (MB/s) | Ratio (%) |
| :--       |           --: |          --: |       --: |
| Encode    |       548.727 |      173.798 |    64.893 |
| Decode    |       809.182 |       76.481 |   154.099 |

### [Base64 Coder](/fly/coders/base64)

Compression ratios of course do not matter with Base64 coding; they will always be 4/3 for encoding
and 3/4 for decoding.

| Direction | Duration (ms) | Speed (MB/s) | Ratio (%) |
| :--       |           --: |          --: |       --: |
| Encode    |       214.870 |      443.837 |   133.333 |
| Decode    |       288.017 |      441.490 |    75.000 |

## Profile

### Huffman Coder

A profile of the Huffman encoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 53.06      0.26     0.26      382     0.68     0.86  fly::HuffmanEncoder::encode_symbols(unsigned int, fly::BitStreamWriter&)
 24.49      0.38     0.12      382     0.31     0.34  fly::HuffmanEncoder::create_tree(unsigned int)
 14.29      0.45     0.07  8111646     0.00     0.00  fly::BitStreamWriter::flush_buffer()
  2.04      0.46     0.01    69915     0.00     0.00  fly::HuffmanEncoder::insert_code(fly::HuffmanCode&&)
  2.04      0.47     0.01    69533     0.00     0.00  fly::HuffmanNode::become_intermediate(fly::HuffmanNode*, fly::HuffmanNode*)
  2.04      0.48     0.01      382     0.03     0.08  fly::HuffmanEncoder::create_codes()
  2.04      0.49     0.01      382     0.03     0.03  fly::HuffmanEncoder::limit_code_lengths()
  0.00      0.49     0.00  3466383     0.00     0.00  fly::HuffmanCode::operator=(fly::HuffmanCode&&)
  0.00      0.49     0.00  3394927     0.00     0.00  fly::operator<(fly::HuffmanCode const&, fly::HuffmanCode const&)
  0.00      0.49     0.00  1187190     0.00     0.00  fly::HuffmanNodeComparator::operator()(fly::HuffmanNode const*, fly::HuffmanNode const*)
  0.00      0.49     0.00    98304     0.00     0.00  fly::HuffmanCode::HuffmanCode()
  0.00      0.49     0.00    70299     0.00     0.00  fly::BitStreamWriter::write_byte(unsigned char)
  0.00      0.49     0.00    69915     0.00     0.00  fly::HuffmanCode::HuffmanCode(fly::HuffmanCode&&)
  0.00      0.49     0.00    69915     0.00     0.00  fly::HuffmanCode::HuffmanCode(unsigned char, unsigned short, unsigned char)
  0.00      0.49     0.00    69915     0.00     0.00  fly::HuffmanNode::become_symbol(unsigned char, unsigned long)
```

A profile of the Huffman decoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 91.30      0.63     0.63      382     1.65     1.81  fly::HuffmanDecoder::decode_symbols(fly::BitStreamReader&, unsigned char, unsigned int, std::ostream&) const
  8.70      0.69     0.06  9792072     0.00     0.00  fly::BitStreamReader::refill_buffer()
  0.00      0.69     0.00    70299     0.00     0.00  fly::BitStreamReader::read_byte(unsigned char&)
  0.00      0.69     0.00    69915     0.00     0.00  fly::HuffmanCode::HuffmanCode(fly::HuffmanCode&&)
  0.00      0.69     0.00    69915     0.00     0.00  fly::HuffmanCode::HuffmanCode(unsigned char, unsigned short, unsigned char)
  0.00      0.69     0.00    69915     0.00     0.00  fly::HuffmanCode::operator=(fly::HuffmanCode&&)
  0.00      0.69     0.00     4585     0.00     0.00  fly::BitStreamReader::read_word(unsigned short&)
  0.00      0.69     0.00     2560     0.00     0.00  fly::HuffmanCode::HuffmanCode()
  0.00      0.69     0.00     1194     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.69     0.00     1159     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.69     0.00     1009     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.69     0.00      512     0.00     0.00  fly::HuffmanNode::HuffmanNode()
  0.00      0.69     0.00      384     0.00     0.00  fly::BitStreamReader::fully_consumed() const
  0.00      0.69     0.00      382     0.00     0.00  fly::HuffmanDecoder::decode_codes(fly::BitStreamReader&, unsigned char&)
```

### Base64 Coder

A profile of the Base64 encoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 81.82      0.09     0.09 33333334     0.00     0.00  fly::Base64Coder::encode_chunk(char const*, char*) const
 18.18      0.11     0.02        1    20.00   110.00  fly::Base64Coder::encode_internal(std::istream&, std::ostream&)
  0.00      0.11     0.00     1194     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.11     0.00     1159     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.11     0.00     1009     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.11     0.00      140     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.11     0.00      117     0.00     0.00  void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)
  0.00      0.11     0.00       87     0.00     0.00  void fly::detail::BasicStringStreamer<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::stream_char<char>(std::ostream&, char)
```

A profile of the Base64 decoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 72.73      0.08     0.08 33333334     0.00     0.00  fly::Base64Coder::decode_chunk(char const*, char*) const
 27.27      0.11     0.03        1    30.00   110.00  fly::Base64Coder::decode_internal(std::istream&, std::ostream&)
  0.00      0.11     0.00     1194     0.00     0.00  std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release()
  0.00      0.11     0.00     1159     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.11     0.00     1009     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::vector(std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > const&)
  0.00      0.11     0.00      140     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.11     0.00      117     0.00     0.00  void std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_construct<char const*>(char const*, char const*, std::forward_iterator_tag)
  0.00      0.11     0.00       83     0.00     0.00  void fly::detail::BasicStringStreamer<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >::stream_char<char>(std::ostream&, char)
```
