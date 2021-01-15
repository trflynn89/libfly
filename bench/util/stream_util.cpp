#include "bench/util/stream_util.hpp"

namespace fly::benchmark {

//==================================================================================================
std::ios::char_type CommaPunctuation::do_thousands_sep() const
{
    return ',';
}

//==================================================================================================
std::string CommaPunctuation::do_grouping() const
{
    return "\3";
}

} // namespace fly::benchmark
