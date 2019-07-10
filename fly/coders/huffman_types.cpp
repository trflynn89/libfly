#include "fly/coders/huffman_types.h"

#include <iostream>

namespace fly {

//==============================================================================
HuffmanNode::HuffmanNode() noexcept :
    m_symbol(0),
    m_frequency(0),
    m_left(nullptr),
    m_right(nullptr)
{
}

//==============================================================================
void HuffmanNode::BecomeSymbol(
    symbol_type symbol,
    frequency_type frequency) noexcept
{
    m_symbol = symbol;
    m_frequency = frequency;
}

//==============================================================================
void HuffmanNode::BecomeIntermediate(
    HuffmanNode *left,
    HuffmanNode *right) noexcept
{
    m_frequency = left->m_frequency + right->m_frequency;
    m_left = left;
    m_right = right;
}

//==============================================================================
bool HuffmanNodeComparator::
operator()(const HuffmanNode *left, const HuffmanNode *right) noexcept
{
    return left->m_frequency > right->m_frequency;
}

//==============================================================================
HuffmanCode::HuffmanCode() noexcept : m_symbol(0), m_code(0), m_length(0)
{
}

//==============================================================================
HuffmanCode::HuffmanCode(
    const symbol_type symbol,
    const code_type code,
    const code_type length) noexcept :
    m_symbol(symbol),
    m_code(code),
    m_length(length)
{
}

} // namespace fly
