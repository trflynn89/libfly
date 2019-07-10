#include "fly/coders/huffman_types.h"

#include <iostream>

namespace fly {

//==============================================================================
HuffmanNode::HuffmanNode() :
    m_symbol(0),
    m_frequency(0),
    m_left(nullptr),
    m_right(nullptr)
{
}

//==============================================================================
bool HuffmanNodeComparator::
operator()(const HuffmanNode *left, const HuffmanNode *right)
{
    return left->m_frequency > right->m_frequency;
}

//==============================================================================
HuffmanCode::HuffmanCode() : m_symbol(0), m_code(0), m_length(0)
{
}

//==============================================================================
HuffmanCode::HuffmanCode(
    const symbol_type symbol,
    const code_type code,
    const code_type length) :
    m_symbol(symbol),
    m_code(code),
    m_length(length)
{
}

} // namespace fly
