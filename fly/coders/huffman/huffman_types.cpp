#include "fly/coders/huffman/huffman_types.hpp"

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
HuffmanNode &HuffmanNode::operator=(HuffmanNode &&node) noexcept
{
    m_symbol = std::move(node.m_symbol);
    m_frequency = std::move(node.m_frequency);
    m_left = node.m_left;
    m_right = node.m_right;

    return *this;
}

//==============================================================================
void HuffmanNode::become_symbol(
    symbol_type symbol,
    frequency_type frequency) noexcept
{
    m_symbol = symbol;
    m_frequency = frequency;
    m_left = nullptr;
    m_right = nullptr;
}

//==============================================================================
void HuffmanNode::become_intermediate(
    HuffmanNode *left,
    HuffmanNode *right) noexcept
{
    m_symbol = 0;
    m_frequency = left->m_frequency + right->m_frequency;
    m_left = left;
    m_right = right;
}

//==============================================================================
bool HuffmanNodeComparator::operator()(
    const HuffmanNode *left,
    const HuffmanNode *right) noexcept
{
    return left->m_frequency > right->m_frequency;
}

//==============================================================================
HuffmanCode::HuffmanCode() noexcept : m_symbol(0), m_code(0), m_length(0)
{
}

//==============================================================================
HuffmanCode::HuffmanCode(
    symbol_type symbol,
    code_type code,
    length_type length) noexcept :
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
bool operator<(const HuffmanCode &left, const HuffmanCode &right)
{
    if (left.m_length == right.m_length)
    {
        return left.m_symbol < right.m_symbol;
    }

    return left.m_length < right.m_length;
}

} // namespace fly
