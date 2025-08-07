#ifndef TWISTYPUZZLE_H
#define TWISTYPUZZLE_H
#include <cstdint>
#include <vector>

class TwistyPuzzle {
public:
	using Row = __uint128_t;
	using FastInt = int_fast32_t;

private:
	Row top;
	Row bottom;

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
	virtual ~TwistyPuzzle() = default;

	/**
	 * @brief Wraps a number of turns to a positive range `eg [0, 18)`
	 *
	 * @param turns the number of turns to wrap
	 * @return the equivalent number of turns
	 */
	[[nodiscard]] virtual int positiveWrap(int turns) const = 0;

	/**
	 * @brief Wraps a number of turns to be centered at zero `eg (-8, 9]`
	 *
	 * @param turns the number of turns to wrap
	 * @return the equivalent number of turns
	 */
	[[nodiscard]] virtual int medianWrap(int turns) const;

	/**
	 *
	 * @brief Encodes a top and bottom turn into a single integer
	 *
	 * @param topTurns the number of top turns
	 * @param bottomTurns the number of bottom turns
	 * @return an encoded integer
	 */
	[[nodiscard]] virtual FastInt encodeMove(FastInt topTurns, FastInt bottomTurns) const;

	/**
	 *
	 * @brief Decodes an encoded integer into a move pair
	 *
	 * @param move encoded move to decode
	 * @return two integers for the top and bottom moves
	 */
	[[nodiscard]] virtual std::pair<FastInt, FastInt> decodeMove(FastInt move) const;

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
	virtual void turn(int topTurns, int bottomTurns);

	/**
	 * @brief Performs a turn followed by a slice move on the puzzle, recording it to a list.
	 *
	 * Mainly for convenience.
	 */
	virtual void move(std::vector<FastInt> &moves, int topTurns, int bottomTurns);

	/**
	 * @brief Performs a turn followed by a slice move on the puzzle.
	 *
	 * Mainly for convenience.
	 */
	virtual void move(int topTurns, int bottomTurns);

	/**
	 * @brief Performs a slice move on the puzzle.
	 *
	 * Swaps the right halves of the top and bottom rows. See Labeled Face Diagram
	 *
	 * @throws logic_error Cannot perform a slice operation if a slice move is currently unavailable.
	 */
	virtual void slice();

	/**
	 * @brief Checks if the puzzle is in cube shape.
	 *
	 * "Cube Shape" refers to whether or not the puzzle contains the same geometry as the solved state.
	 * This only checks if the shape matches, Not orientation nor Piece ID.
	 *
	 * @return TRUE if both the top and bottom rows are in proper cube shape.
	 */
	[[nodiscard]] virtual bool cubeShape() const;

	/**
	* @brief Checks if a slice move is currently allowed.
	 *
	 * A slice move is only permitted when both the top and bottom rows do not have any obstructing pieces along the slice axis.
	 *
	 * @return TRUE if both layers are allowed to be sliced.
	 */
	[[nodiscard]] virtual bool canSlice() const;

	/**
	 * @brief Checks if the top row can be sliced.
	 * @return TRUE if the top layer can be sliced.
	 */
	[[nodiscard]] virtual bool canSliceTop() const;

	/**
	 * @brief Checks if the bottom row can be sliced.
	 * @return TRUE if the bottom layer can be sliced.
	 */
	[[nodiscard]] virtual bool canSliceBottom() const;

	/**
	 * @brief Checks if all the top and bottom pieces are in the correct row.
	 * @return TRUE if all the top pieces are in the top row, and all the bottom pieces are in the bottom row.
	 */
	[[nodiscard]] virtual bool isRowOrientationSolved() const;

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
	[[nodiscard]] virtual bool isSolvedByMatches(Row topMatch, Row topMask, Row bottomMatch, Row bottomMask) const;

	/**
	 * @brief Checks if the puzzle is solved.
	 * @return TRUE if the top and bottom rows match the solved state.
	 */
	[[nodiscard]] virtual bool isSolved() const;

	/**
	 * @brief Checks if the top row is solved.
	 * @return TRUE if the top row matches the solved state.
	 */
	[[nodiscard]] virtual bool isTopSolved() const;

	/**
	 * @brief Checks if the bottom row is solved.
	 * @return TRUE if the bottom row matches the solved state.
	 */
	[[nodiscard]] virtual bool isBottomSolved() const;

	/**
	 * @brief Clones the puzzle
	 * @return A true clone
	 */
	[[nodiscard]] virtual TwistyPuzzle clone() const;

	/**
	 * @brief Prints the contents of a puzzle.
	 *
	 * This prints the information you might need at a quick glance.
	 * This includes Is Cube Shape, Can Slice, Is Row Orientation Solved, and Is Solved.
	 * As well as the two rows themselves.
	 *
	 */
	virtual void print() const;
};

#endif //TWISTYPUZZLE_H
