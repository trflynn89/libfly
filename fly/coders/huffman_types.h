#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

namespace fly {

typedef std::filesystem::path::value_type symbol_type;
typedef std::uint64_t frequency_type;
typedef std::uint8_t code_type;

/**
 * Struct to store data for a single node in a Huffman tree. Huffman trees are
 * binary trees. A node represents either a symbol from the input stream and its
 * frequency, or the node is a junction storing the sum total of the frequencies
 * of each node in its subtree.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
struct HuffmanNode
{
    /**
     * Default constructor. Set all fields to zero.
     */
    HuffmanNode();

    /**
     * Constructor for a symbol node.
     *
     * @param symbol_type The symbol from the input stream.
     * @param frequency_type The frequency of the symbol in the input stream.
     */
    HuffmanNode(const symbol_type, const frequency_type);

    /**
     * Constructor for a junction node. This node's frequency is set to the sum
     * of the children's frequencies.
     *
     * @return unique_ptr Scoped pointer to the left child of this junction.
     * @return unique_ptr Scoped pointer to the right child of this junction.
     */
    HuffmanNode(std::unique_ptr<HuffmanNode>, std::unique_ptr<HuffmanNode>);

    /**
     * Determine if this node is a symbol node by checking it is has children.
     *
     * @return bool True if this node is a symbol node.
     */
    bool IsSymbol() const;

    /**
     * TODO remove.
     */
    void Print(int depth = 0);

    symbol_type m_symbol;
    frequency_type m_frequency;

    std::unique_ptr<HuffmanNode> m_left;
    std::unique_ptr<HuffmanNode> m_right;
};

/**
 * Struct to store data for a Huffman code.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
struct HuffmanCode
{
    /**
     * Constructor.
     *
     * @param symbol_type The symbol from the input stream.
     * @param code_type The Huffman code for the symbol.
     * @param code_type The number of bits in the Huffman code.
     */
    HuffmanCode(const symbol_type, const code_type, const code_type);

    symbol_type m_symbol;
    code_type m_code;
    code_type m_length;
};

} // namespace fly
