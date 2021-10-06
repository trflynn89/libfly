#include "fly/coders/huffman/types.hpp"

namespace fly::coders {

//==================================================================================================
HuffmanNode::HuffmanNode() noexcept : m_symbol(0), m_frequency(0), m_left(nullptr), m_right(nullptr)
{
}

//==================================================================================================
HuffmanNode &HuffmanNode::operator=(HuffmanNode &&node) noexcept
{
    m_symbol = node.m_symbol;
    m_frequency = node.m_frequency;
    m_left = node.m_left;
    m_right = node.m_right;

    node.m_left = nullptr;
    node.m_right = nullptr;

    return *this;
}

//==================================================================================================
void HuffmanNode::become_symbol(symbol_type symbol, frequency_type frequency)
{
    m_symbol = symbol;
    m_frequency = frequency;
    m_left = nullptr;
    m_right = nullptr;
}

//==================================================================================================
void HuffmanNode::become_intermediate(HuffmanNode *left, HuffmanNode *right)
{
    m_symbol = 0;
    m_frequency = left->m_frequency + right->m_frequency;
    m_left = left;
    m_right = right;
}

//==================================================================================================
bool HuffmanNodeComparator::operator()(const HuffmanNode *left, const HuffmanNode *right)
{
    return left->m_frequency > right->m_frequency;
}

//==================================================================================================
HuffmanCode::HuffmanCode() noexcept : m_symbol(0), m_code(0), m_length(0)
{
}

//==================================================================================================
HuffmanCode::HuffmanCode(symbol_type symbol, code_type code, length_type length) noexcept :
    m_symbol(symbol),
    m_code(code),
    m_length(length)
{
}

//==================================================================================================
HuffmanCode::HuffmanCode(HuffmanCode &&code) noexcept :
    m_symbol(code.m_symbol),
    m_code(code.m_code),
    m_length(code.m_length)
{
}

//==================================================================================================
HuffmanCode &HuffmanCode::operator=(HuffmanCode &&code) noexcept
{
    m_symbol = code.m_symbol;
    m_code = code.m_code;
    m_length = code.m_length;

    return *this;
}

//==================================================================================================
bool operator<(const HuffmanCode &left, const HuffmanCode &right)
{
    if (left.m_length == right.m_length)
    {
        return left.m_symbol < right.m_symbol;
    }

    return left.m_length < right.m_length;
}

} // namespace fly::coders
