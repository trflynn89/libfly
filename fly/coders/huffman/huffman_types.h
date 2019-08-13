#pragma once

#include <cstdint>
#include <queue>
#include <vector>

namespace fly {

typedef std::uint8_t symbol_type;
typedef std::uint64_t frequency_type;

typedef std::uint16_t code_type;
typedef std::uint8_t length_type;

struct HuffmanNode;
struct HuffmanNodeComparator;

typedef std::priority_queue<
    HuffmanNode *,
    std::vector<HuffmanNode *>,
    HuffmanNodeComparator>
    HuffmanNodeQueue;

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
     * Default constructor. Set all fields to zero/null.
     */
    HuffmanNode() noexcept;

    /**
     * Deleted copy constructor.
     */
    HuffmanNode(const HuffmanNode &) = delete;

    /**
     * Move constructor. Move all member variables from the given HuffmanNode
     * instance into this instance.
     *
     * @param HuffmanNode The HuffmanNode instance to move.
     */
    HuffmanNode(HuffmanNode &&) noexcept;

    /**
     * Deleted assignment operator.
     */
    HuffmanNode &operator=(const HuffmanNode &) = delete;

    /**
     * Move assignment operator. Move all member variables from the given
     * HuffmanNode instance into this instance.
     *
     * @param HuffmanNode The HuffmanNode instance to move.
     */
    HuffmanNode &operator=(HuffmanNode &&) noexcept;

    /**
     * Change this node to represent a symbol from the input stream.
     *
     * @param symbol_type The symbol from the input stream.
     * @param frequency_type The frequency of the symbol in the input stream.
     */
    void BecomeSymbol(symbol_type, frequency_type) noexcept;

    /**
     * Change this node to represent an intermediate, non-symbol. Its frequency
     * is set to the sum of its children's frequencies.
     *
     * @param HuffmanNode Pointer to the intermediate's left child.
     * @param HuffmanNode Pointer to the intermediate's right child.
     */
    void BecomeIntermediate(HuffmanNode *, HuffmanNode *) noexcept;

    symbol_type m_symbol;
    frequency_type m_frequency;

    HuffmanNode *m_left;
    HuffmanNode *m_right;
};

/**
 * Comparator for HuffmanNode to be sorted such that the node with the lowest
 * frequency has the highest priority.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
struct HuffmanNodeComparator
{
    bool operator()(const HuffmanNode *left, const HuffmanNode *right) noexcept;
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
     * Default constructor. Set all fields to zero.
     */
    HuffmanCode() noexcept;

    /**
     * Constructor.
     *
     * @param symbol_type The symbol from the input stream.
     * @param code_type The Huffman code for the symbol.
     * @param length_type The number of bits in the Huffman code.
     */
    HuffmanCode(const symbol_type, const code_type, const length_type) noexcept;

    /**
     * Deleted copy constructor.
     */
    HuffmanCode(const HuffmanCode &) = delete;

    /**
     * Move constructor. Move all member variables from the given HuffmanCode
     * instance into this instance.
     *
     * @param HuffmanCode The HuffmanCode instance to move.
     */
    HuffmanCode(HuffmanCode &&) noexcept;

    /**
     * Deleted assignment operator.
     */
    HuffmanCode &operator=(const HuffmanCode &) = delete;

    /**
     * Move assignment operator. Move all member variables from the given
     * HuffmanCode instance into this instance.
     *
     * @param HuffmanCode The HuffmanCode instance to move.
     */
    HuffmanCode &operator=(HuffmanCode &&) noexcept;

    /**
     * Less-than operator. Huffman codes are first compared by code length, and
     * then by symbol value.
     *
     * @param HuffmanCode The first Huffman code to compare.
     * @param HuffmanCode The second Huffman code to compare.
     *
     * @return bool True if the first Huffman code is less than the second.
     */
    friend bool operator<(const HuffmanCode &, const HuffmanCode &);

    symbol_type m_symbol;
    code_type m_code;
    length_type m_length;
};

} // namespace fly
