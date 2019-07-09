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
HuffmanNode::HuffmanNode(
    const symbol_type symbol,
    const frequency_type frequency) :
    m_symbol(symbol),
    m_frequency(frequency),
    m_left(nullptr),
    m_right(nullptr)
{
}

//==============================================================================
HuffmanNode::HuffmanNode(
    std::unique_ptr<HuffmanNode> left,
    std::unique_ptr<HuffmanNode> right) :
    m_symbol(0),
    m_frequency(left && right ? left->m_frequency + right->m_frequency : 0),
    m_left(std::move(left)),
    m_right(std::move(right))
{
}

//==============================================================================
bool HuffmanNode::IsSymbol() const
{
    return (m_left == nullptr) && (m_right == nullptr);
}

//==============================================================================
void HuffmanNode::Print(int depth)
{
    if (depth > 0)
    {
        std::cout << '|';

        for (int i = 0; i < depth; ++i)
        {
            std::cout << "-";
        }
    }

    if (m_symbol)
    {
        std::cout << m_symbol << " (" << m_frequency << ")\n";
    }
    else
    {
        std::cout << "[" << m_frequency << "]\n";
    }

    if (m_left)
    {
        m_left->Print(depth + 1);
    }
    if (m_right)
    {
        m_right->Print(depth + 1);
    }
}

//==============================================================================
HuffmanTable::HuffmanTable() : m_symbol(0), m_left(nullptr), m_right(nullptr)
{
}

//==============================================================================
bool HuffmanTable::IsSymbol() const
{
    return (m_left == nullptr);
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
