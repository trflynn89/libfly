#include "fly/coders/huffman/huffman_types.h"

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
HuffmanNode::HuffmanNode(HuffmanNode &&node) noexcept :
    m_symbol(std::move(node.m_symbol)),
    m_frequency(std::move(node.m_frequency)),
    m_left(node.m_left),
    m_right(node.m_right)
{
}

//==============================================================================
HuffmanNode &HuffmanNode::operator=(HuffmanNode &&node) noexcept
{
    m_symbol = std::move(node.m_symbol);
    m_frequency = std::move(node.m_frequency);
    m_left = node.m_left;
    m_right = node.m_right;

    return *this;
}

//==============================================================================
void HuffmanNode::BecomeSymbol(
    symbol_type symbol,
    frequency_type frequency) noexcept
{
    m_symbol = symbol;
    m_frequency = frequency;
    m_left = nullptr;
    m_right = nullptr;
}

//==============================================================================
void HuffmanNode::BecomeIntermediate(
    HuffmanNode *left,
    HuffmanNode *right) noexcept
{
    m_symbol = 0;
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
    const length_type length) noexcept :
    m_symbol(symbol),
    m_code(code),
    m_length(length)
{
}

//==============================================================================
HuffmanCode::HuffmanCode(HuffmanCode &&code) noexcept :
    m_symbol(std::move(code.m_symbol)),
    m_code(std::move(code.m_code)),
    m_length(std::move(code.m_length))
{
}

//==============================================================================
HuffmanCode &HuffmanCode::operator=(HuffmanCode &&code) noexcept
{
    m_symbol = std::move(code.m_symbol);
    m_code = std::move(code.m_code);
    m_length = std::move(code.m_length);

    return *this;
}

//==============================================================================
bool operator<(const HuffmanCode &a, const HuffmanCode &b)
{
    if (a.m_length == b.m_length)
    {
        return a.m_symbol < b.m_symbol;
    }

    return a.m_length < b.m_length;
}

} // namespace fly
