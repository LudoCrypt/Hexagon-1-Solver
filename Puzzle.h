#ifndef PUZZLE_H
#define PUZZLE_H
#include <cstdint>
#include <vector>

/**
 * @class Puzzle
 *
 * @brief A mutable binary representation of a Hexagon-1 twisty puzzle.
 *
 * A single face is modeled using a 128-bit integer (`Row`).
 * This is a compressed int of 18 contiguous slots containing 6 bits per slot.
 * Each 6-bit slot encodes exactly one uniquely identifiable piece, along with relevant information. (See Binary Slot Format).
 *
 * ──────────────────────────────────────────────────────────────────────────────────────────────────
 *
 * Binary Slot Format
 *
 * Slot Flow Chart:
 *
 *   Bit Index:   [0]   [1]   [2     3     4     5]
 *   Bit Value:    0     1     0     0     1     1
 *                ─┬─────┬─────┬─────┬─────┬─────┬─
 *                 │     │     ├─────╯─────╯─────┤
 *                 │     │     │                 ╰─► [5] Corner Flag
 *                 │     │     ╰───────────────────► [2-5] Piece ID (4 bits)
 *                 │     ╰─────────────────────────► [1] Corner Parity (0 = Left, 1 = Right)
 *                 ╰───────────────────────────────► [0] Face Parity (0 = Top, 1 = Bottom)
 *
 * Bit Specifics:
 *
 *   [0] Face Parity:
 *       - 0: Top Face (White side)
 *       - 1: Bottom Face (Yellow side)
 *
 *   [1] Corner Parity:
 *       - 0: Left half of a corner
 *       - 1: Right half of a corner
 *
 *   [2-5] Piece ID:
 *       - Interpreted as a 4-bit binary number, e.g. 0011 = C2
 *       - Both halves of a corner share a Piece ID
 *
 *   [5] Corner Flag:
 *       - 0: Is an edge
 *       - 1: Not an edge
 *
 *   Note: Bit [5] is still a part of the Piece ID, but the IDs are arranged such that this is always a 0 for an edge.
 *
 * Example:
 *   Binary: 010011
 *     - [0] = 0 → Top (White) Face
 *     - [1] = 1 → Right corner
 *     - [2-5] = 0011 → C2
 *     - [5] = 1 → This is not an edge.
 *
 * The solved state of the puzzle (as interpreted literally) is as follows:
 * Top    → 00000000000000000000 000001 010001 000010 000011 010011 000100 000101 010101 000110 000111 010111 001000 001001 011001 001010 001011 011011 001100
 * Bottom → 00000000000000000000 100110 100101 110101 100100 100011 110011 100010 100001 110001 101100 101011 111011 101010 101001 111001 101000 100111 110111
 *
 * ──────────────────────────────────────────────────────────────────────────────────────────────────
 *
 * Puzzle Pieces
 *
 * The solved state of the puzzle starts from the back right of the slice, labels alternating C1 E1 for corners and edges respectively.
 * Note that corners actually use up two slots, but share one Piece ID. See bit [1].
 *
 * Labeled Face Diagram:
 *                                ╭────► Odd ID (Corner)
 *                           000001      │
 *                             ▼     ╭───┼─► Right Half
 *                    C6  E6 / C1 ◄ 010001
 *                  E5      /    E1
 *   Left Half    C5       •       C2    Right Half
 *                  E4    /      E2
 *                    C4 / E3  C3
 *                         ▲
 *                      000110
 *                           ╰────► Even (Edge)
 *
 * Note that every instance of, for example, E3, will share the same Puzzle ID of 0110.
 * The E3 slot on the bottom face is 100110, but because it's still E3 it's ID is 0110.
 *
 * The beginning of the top row begins at C1 and runs clockwise.
 * The beginning of the bottom row begins at E3 and runs counterclockwise.
 *
 * This is done to greatly simplify the logic of the slice operation.
 *
 * ──────────────────────────────────────────────────────────────────────────────────────────────────
 */
class Puzzle {
public:
	using Row = __uint128_t;

	// The number of bits that a slot takes up
	static constexpr int SLOT_SIZE = 6;
	// The number of slots per row
	static constexpr int SLOTS_PER_ROW = 18;
	// The number of slots per half of the puzzle
	static constexpr int SLOTS_PER_HALF = (SLOTS_PER_ROW / 2);
	// The number of bits that a row takes up
	static constexpr int ROW_BITS = SLOTS_PER_ROW * SLOT_SIZE;
	// The number of bits that the row fits into (including ignored space)
	static constexpr int TOTAL_BITS = 128;
	// The number of bits that make up half a row
	static constexpr int HALF_BITS = (SLOTS_PER_ROW / 2) * SLOT_SIZE;

	// Mask that isolates out a single piece
	static constexpr Row SLOT_MASK = (static_cast<Row>(1) << SLOT_SIZE) - 1;

	// Mask that isolates out a single row
	static constexpr Row ROW_MASK = (static_cast<Row>(1) << ROW_BITS) - 1;

	// Initial starting position for the top
	static constexpr Row SOLVED_TOP = static_cast<Row>(0x000000510834C415ULL) << 64 | 0x51875C825928B6CCULL;

	// Initial starting position for the bottom
	static constexpr Row SOLVED_BOTTOM = static_cast<Row>(0x000009a5d648f38aULL) << 64 | 0x1c6cafbaa9e689f7ULL;

private:
	Row top;
	Row bottom;

	// Mask that isolates the right half of the slots. See slice()
	static constexpr Row HALF_MASK = ((static_cast<Row>(1) << HALF_BITS) - 1) << (ROW_BITS - HALF_BITS);

	// Mask to check if the top is in proper cube shape. See cubeShape()
	static constexpr Row TOP_CUBE_SHAPE = static_cast<Row>(0x0000000004000100ULL) << 64 | 0x0040001000040001ULL;

	// Mask to check if the bottom is in proper cube shape. See cubeShape()
	static constexpr Row BOTTOM_CUBE_SHAPE = static_cast<Row>(0x0000004000100004ULL) << 64 | 0x0001000040001000ULL;

	// Mask to check if a slice move is available. See canSlice()
	static constexpr Row SLICE_MASK = static_cast<Row>(0x0000040000000000ULL) << 64 | 0x0010000000000000ULL;

	// Mask to check if the top and bottom faces are solved. See isRowOrientationSolved()
	static constexpr Row ROW_ORIENTATION_MASK = static_cast<Row>(0x0000082082082082ULL) << 64 | 0x0820820820820820ULL;

	/**
	 * Applies a circular rotation to a specific row.
	 * Rotation is clockwise for positive values.
	 *
	 * @param row The 128-bit row to rotate.
	 * @param slots The number of slots to rotate by.
	 * @return The rotated row.
	 *
	 * Example:
	 *   - Input:  row = [C1, E1, C2, ..., E6] (18 slots, 12 pieces), slots = 4
	 *                   ─┬───┬─          ─┬─
	 *                    ╰───┼────────╮   │
	 *                        │    ╭───┼───╯
	 *                        ╰────┼───┼───╮
	 *                             ▼   ▼   ▼
	 *   - Output:  row = [E5, C6, E6, C1, E1, ..., C5]
	 *
	 */
	static Row turnRow(Row row, int slots);

public:
	Puzzle();

	Puzzle(Row, Row);

	/**
	 * @brief Wraps a number of turns to the range [0, 18)
	 *
	 * @param turns the number of turns to wrap
	 * @return a number wrapped to the range [0, 18)
	 */
	static int wrapPositive(int turns);

	/**
	 * @brief Wraps a number of turns to the range (-8, 9]
	 *
	 * @param turns the number of turns to wrap
	 * @return a number wrapped to the range (-8, 9]
	 */
	static int wrapNegative(int turns);

	/**
	 *
	 * @brief Encodes a top and bottom turn into a single integer
	 *
	 * @param topTurns the number of top turns
	 * @param bottomTurns the number of bottom turns
	 * @return an encoded integer
	 */
	static int_fast32_t encodeMove(int_fast32_t topTurns, int_fast32_t bottomTurns);

	/**
	 *
	 * @brief Decodes an encoded integer into a move pair
	 *
	 * @param move encoded move to decode
	 * @return two integers for the top and bottom moves
	 */
	static std::pair<int_fast32_t, int_fast32_t> decodeMove(int_fast32_t move);

	/**
	 *
	 * @brief Performs a rotation on the top and bottom rows.
	 *
	 * Rotation is clockwise for positive values.
	 *
	 * @param topTurns Number of slots to rotate the top row.
	 * @param bottomTurns Number of slots to rotate the bottom row.
	 *
	 * Example:
	 *   - topTurns = 2: top row is rotated 2 slots clockwise.
	 *   - bottomTurns = -1: bottom row is rotated 1 slot counterclockwise.
	 */
	void turn(int topTurns, int bottomTurns);

	/**
	 * @brief Performs a turn followed by a slice move on the puzzle, recording it to a list.
	 *
	 * Mainly for convenience.
	 *
	 */
	void move(std::vector<int_fast32_t> &moves, int topTurns, int bottomTurns);

	/**
	 * @brief Performs a turn followed by a slice move on the puzzle.
	 *
	 * Mainly for convenience.
	 *
	 */
	void move(int topTurns, int bottomTurns);

	/**
	 * @brief Performs a slice move on the puzzle.
	 *
	 * Swaps the right halves of the top and bottom rows. See Labeled Face Diagram
	 *
	 * @throws logic_error Cannot perform a slice operation if a slice move is currently unavailable.
	 */
	void slice();

	/**
	 * @brief Checks if the puzzle is in cube shape.
	 *
	 * "Cube Shape" refers to whether or not the puzzle contains the same geometry as the solved state.
	 * This only checks if the shape matches, Not orientation nor Piece ID.
	 *
	 * @return TRUE if both the top and bottom rows are in proper cube shape.
	 */
	[[nodiscard]] bool cubeShape() const;

	/**
	* @brief Checks if a slice move is currently allowed.
	 *
	 * A slice move is only permitted when both the top and bottom rows do not have any obstructing pieces along the slice axis.
	 *
	 * @return TRUE if both layers are allowed to be sliced.
	 */
	[[nodiscard]] bool canSlice() const;

	/**
	 * @brief Checks if the top row can be sliced.
	 * @return TRUE if the top layer can be sliced.
	 */
	[[nodiscard]] bool canSliceTop() const;

	/**
	 * @brief Checks if the bottom row can be sliced.
	 * @return TRUE if the bottom layer can be sliced.
	 */
	[[nodiscard]] bool canSliceBottom() const;

	/**
	 * @brief Checks if all the top and bottom pieces are in the correct row.
	 * @return TRUE if all the top pieces are in the top row, and all the bottom pieces are in the bottom row.
	 */
	[[nodiscard]] bool isRowOrientationSolved() const;

	/**
	 * @brief Checks if the puzzle matches a specific layout.
	 *
	 * This is meant to be used to check if a specific set of pieces are in the right order, while ignoring other pieces.
	 *
	 * @param topMatch The encoded row that the top row must match.
	 * @param topMask The mask to be used for ignoring certain pieces on the top row.
	 * @param bottomMatch The encoded row that the bottom row must match.
	 * @param bottomMask The mask to be used for ignoring certain pieces on the bottom row.
	 *
	 * @return TRUE if the top row and bottom rows match where required.
	 */
	[[nodiscard]] bool isSolvedByMatches(Row topMatch, Row topMask, Row bottomMatch, Row bottomMask) const;

	/**
	 * @brief Checks if the puzzle is solved.
	 * @return TRUE if the top and bottom rows match the solved state.
	 */
	[[nodiscard]] bool isSolved() const;

	/**
	 * @brief Checks if the top row is solved.
	 * @return TRUE if the top row matches the solved state.
	 */
	[[nodiscard]] bool isTopSolved() const;

	/**
	 * @brief Checks if the bottom row is solved.
	 * @return TRUE if the bottom row matches the solved state.
	 */
	[[nodiscard]] bool isBottomSolved() const;

	/**
	 * @brief Clones the puzzle
	 * @return A true clone
	 */
	[[nodiscard]] Puzzle clone() const;

	/**
	 * @brief Prints a row in its binary slot format.
	 */
	static void printRow(Row row);

	/**
	 * @brief Prints the contents of a puzzle.
	 *
	 * This prints the information you might need at a quick glance.
	 * This includes Is Cube Shape, Can Slice, Is Row Orientation Solved, and Is Solved.
	 * As well as the two rows themselves.
	 *
	 */
	void print() const;
};

#endif //PUZZLE_H
