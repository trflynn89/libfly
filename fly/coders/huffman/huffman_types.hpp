#pragma once

#include <cstdint>
#include <queue>
#include <vector>

namespace fly::coders {

using symbol_type = std::uint8_t;
using frequency_type = std::uint64_t;

using code_type = std::uint16_t;
using length_type = std::uint8_t;

struct HuffmanNode;
struct HuffmanNodeComparator;

using HuffmanNodeQueue =
    std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, HuffmanNodeComparator>;

/**
 * Struct to store data for a single node in a Huffman tree. Huffman trees are binary trees. A node
 * represents either a symbol from the input stream and its frequency, or the node is a junction
 * storing the sum total of the frequencies of each node in its subtree.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
struct HuffmanNode
{
    /**
     * Default constructor. Set all fields to zero/null.
     */
    HuffmanNode() noexcept;

    /**
     * Move assignment operator. Move all member variables from the given HuffmanNode instance into
     * this instance.
     *
     * @param node The HuffmanNode instance to move.
     */
    HuffmanNode &operator=(HuffmanNode &&node) noexcept;

    /**
     * Change this node to represent a symbol from the input stream.
     *
     * @param symbol The symbol from the input stream.
     * @param frequency The frequency of the symbol in the input stream.
     */
    void become_symbol(symbol_type symbol, frequency_type frequency);

    /**
     * Change this node to represent an intermediate, non-symbol. Its frequency is set to the sum of
     * its children's frequencies.
     *
     * @param left Pointer to the intermediate's left child.
     * @param right Pointer to the intermediate's right child.
     */
    void become_intermediate(HuffmanNode *left, HuffmanNode *right);

    symbol_type m_symbol;
    frequency_type m_frequency;

    HuffmanNode *m_left;
    HuffmanNode *m_right;

private:
    HuffmanNode(const HuffmanNode &) = delete;
    HuffmanNode &operator=(const HuffmanNode &) = delete;
};

/**
 * Comparator for HuffmanNode to be sorted such that the node with the lowest frequency has the
 * highest priority.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
struct HuffmanNodeComparator
{
    bool operator()(const HuffmanNode *left, const HuffmanNode *right);
};

/**
 * Struct to store data for a Huffman code.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
struct HuffmanCode
{
    /**
     * Default constructor. Set all fields to zero.
     */
    HuffmanCode() noexcept;

    /**
     * Constructor.
     *
     * @param symbol The symbol from the input stream.
     * @param code The Huffman code for the symbol.
     * @param length The number of bits in the Huffman code.
     */
    HuffmanCode(symbol_type symbol, code_type code, length_type length) noexcept;

    /**
     * Move constructor. Move all member variables from the given HuffmanCode instance into this
     * instance.
     *
     * @param code The HuffmanCode instance to move.
     */
    HuffmanCode(HuffmanCode &&code) noexcept;

    /**
     * Move assignment operator. Move all member variables from the given HuffmanCode instance into
     * this instance.
     *
     * @param code The HuffmanCode instance to move.
     */
    HuffmanCode &operator=(HuffmanCode &&code) noexcept;

    /**
     * Less-than operator. Huffman codes are first compared by code length, then by symbol value.
     *
     * @param left The first Huffman code to compare.
     * @param right The second Huffman code to compare.
     *
     * @return True if the first Huffman code is less than the second.
     */
    friend bool operator<(const HuffmanCode &left, const HuffmanCode &right);

    symbol_type m_symbol;
    code_type m_code;
    length_type m_length;

private:
    HuffmanCode(const HuffmanCode &) = delete;
    HuffmanCode &operator=(const HuffmanCode &) = delete;
};

} // namespace fly::coders
