# Coders

Benchmark of the libfly coders with [enwik8](http://mattmahoney.net/dc/enwik8.zip), an approximately
100 MB file. This file is not comitted into git, so it must be fetched, unzipped, and stored in the
[data](/bench/coders/data) folder.

## Results

All results below are the median of 11 iterations of encoding and decoding the enwik8 file.

### [Huffman Coder](/fly/coders/huffman)

| Direction | Duration (ms) | Speed (MB/s) | Ratio (%) |
| :--       |           --: |          --: |       --: |
| Encode    |       261.214 |      365.093 |    64.893 |
| Decode    |       464.504 |      133.232 |   154.099 |


### [Base64 Coder](/fly/coders/base64)

Compression ratios of course do not matter with Base64 coding; they will always be 4/3 for encoding
and 3/4 for decoding.

| Direction | Duration (ms) | Speed (MB/s) | Ratio (%) |
| :--       |           --: |          --: |       --: |
| Encode    |        92.233 |     1033.984 |   133.333 |
| Decode    |        83.447 |     1523.793 |    75.000 |

## Profile

### Huffman Coder

A profile of the Huffman encoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 72.50      1.45     1.45     4202     0.00     0.00  fly::coders::HuffmanEncoder::encode_symbols(unsigned int, fly::BitStreamWriter&)
 19.00      1.83     0.38     4202     0.00     0.00  fly::coders::HuffmanEncoder::create_tree(unsigned int)
  6.00      1.95     0.12 89228106     0.00     0.00  fly::BitStreamWriter::flush_buffer()
  1.00      1.97     0.02   769065     0.00     0.00  fly::coders::HuffmanEncoder::insert_code(fly::coders::HuffmanCode&&)
  0.50      1.98     0.01 38130213     0.00     0.00  fly::coders::HuffmanCode::operator=(fly::coders::HuffmanCode&&)
  0.50      1.99     0.01     4202     0.00     0.00  fly::coders::HuffmanEncoder::create_codes()
  0.50      2.00     0.01     4202     0.00     0.00  fly::coders::HuffmanEncoder::convert_to_canonical_form()
  0.00      2.00     0.00 37344197     0.00     0.00  fly::coders::operator<(fly::coders::HuffmanCode const&, fly::coders::HuffmanCode const&)
  0.00      2.00     0.00 13059090     0.00     0.00  fly::coders::HuffmanNodeComparator::operator()(fly::coders::HuffmanNode const*, fly::coders::HuffmanNode const*)
  0.00      2.00     0.00  1076224     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode()
  0.00      2.00     0.00   773289     0.00     0.00  fly::BitStreamWriter::write_byte(unsigned char)
  0.00      2.00     0.00   769065     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode(fly::coders::HuffmanCode&&)
  0.00      2.00     0.00   769065     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode(unsigned char, unsigned short, unsigned char)
  0.00      2.00     0.00   769065     0.00     0.00  fly::coders::HuffmanNode::become_symbol(unsigned char, unsigned long)
  0.00      2.00     0.00   764863     0.00     0.00  fly::coders::HuffmanNode::become_intermediate(fly::coders::HuffmanNode*, fly::coders::HuffmanNode*)
  0.00      2.00     0.00    50435     0.00     0.00  fly::BitStreamWriter::write_word(unsigned short)
```

A profile of the Huffman decoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 84.21      4.16     4.16     4202     0.00     0.00  fly::coders::HuffmanDecoder::decode_symbols(fly::BitStreamReader&, unsigned char, unsigned int, std::ostream&) const
 15.79      4.94     0.78 107712792     0.00     0.00  fly::BitStreamReader::refill_buffer()
  0.00      4.94     0.00   773289     0.00     0.00  fly::BitStreamReader::read_byte(unsigned char&)
  0.00      4.94     0.00   769065     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode(fly::coders::HuffmanCode&&)
  0.00      4.94     0.00   769065     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode(unsigned char, unsigned short, unsigned char)
  0.00      4.94     0.00   769065     0.00     0.00  fly::coders::HuffmanCode::operator=(fly::coders::HuffmanCode&&)
  0.00      4.94     0.00    50435     0.00     0.00  fly::BitStreamReader::read_word(unsigned short&)
  0.00      4.94     0.00    23040     0.00     0.00  fly::coders::HuffmanCode::HuffmanCode()
  0.00      4.94     0.00     4224     0.00     0.00  fly::BitStreamReader::fully_consumed() const
  0.00      4.94     0.00     4202     0.00     0.00  fly::coders::HuffmanDecoder::decode_codes(fly::BitStreamReader&, unsigned char&)
  0.00      4.94     0.00     4202     0.00     0.00  fly::coders::HuffmanDecoder::convert_to_prefix_table(unsigned char)
```

### Base64 Coder

A profile of the Base64 encoder:

```
100.00      0.49     0.49       11    44.55    44.55  fly::coders::Base64Coder::encode_internal(std::istream&, std::ostream&)
  0.00      0.49     0.00     1488     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::push_back(char)
  0.00      0.49     0.00       54     0.00     0.00  void std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::_M_realloc_insert<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&>(__gnu_cxx::__normal_iterator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >*, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.49     0.00       50     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::push_back(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.49     0.00       35     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.49     0.00       35     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_assign(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
```

A profile of the Base64 decoder:

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
  100.00      0.47     0.47       11    42.73    42.73  fly::coders::Base64Coder::decode_internal(std::istream&, std::ostream&)
  0.00      0.47     0.00     1434     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::push_back(char)
  0.00      0.47     0.00       54     0.00     0.00  void std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::_M_realloc_insert<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&>(__gnu_cxx::__normal_iterator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >*, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.47     0.00       50     0.00     0.00  std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >::push_back(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
  0.00      0.47     0.00       35     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_dispose()
  0.00      0.47     0.00       35     0.00     0.00  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_assign(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
```
