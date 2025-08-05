#include "Puzzle.h"
#include <bitset>
#include <iostream>

Puzzle::Puzzle() : top(0), bottom(0) {
	top = SOLVED_TOP;
	bottom = SOLVED_BOTTOM;
}

Puzzle::Puzzle(const Row topRow, const Row bottomRow) : top(topRow), bottom(bottomRow) {
}

int Puzzle::wrapPositive(const int turns) {
	return ((turns % SLOTS_PER_ROW + SLOTS_PER_ROW) % SLOTS_PER_ROW);
}

int Puzzle::wrapNegative(const int turns) {
	return ((wrapPositive(turns) + SLOTS_PER_HALF - 1) % SLOTS_PER_ROW - (SLOTS_PER_HALF - 1));
}

int_fast32_t Puzzle::encodeMove(const int_fast32_t topTurns, const int_fast32_t bottomTurns) {
	return (wrapPositive(topTurns) << SLOT_SIZE) | wrapPositive(bottomTurns);
}

std::pair<int_fast32_t, int_fast32_t> Puzzle::decodeMove(const int_fast32_t move) {
	return std::make_pair((move >> SLOT_SIZE) & ((1 << SLOT_SIZE) - 1), move & ((1 << SLOT_SIZE) - 1));
}

Puzzle::Row Puzzle::turnRow(const Row row, const int slots) {
	if (slots == 0)
		return row;
	// Normalizes to [0, 18) and multiplies that by 6 to get the number of bits over it's shifting.
	const int shift = wrapPositive(slots) * SLOT_SIZE;
	// Bitshift to the left end except for the last number of slots to clear the beginning bits
	// Then bitshift back to the right 20 bits to move it to the start of the row
	const Row tail = (row << (TOTAL_BITS - shift)) >> (TOTAL_BITS - ROW_BITS);
	// Shift the row to the right, and reapply the missing bits
	Row next = (row >> shift) | tail;
	// Make sure nothing is in the unused space
	next &= ROW_MASK;
	return next;
}

void Puzzle::turn(const int topTurns, const int bottomTurns) {
	top = turnRow(top, topTurns);
	bottom = turnRow(bottom, bottomTurns);
}

void Puzzle::move(const int topTurns, const int bottomTurns) {
	turn(topTurns, bottomTurns);
	slice();
}

void Puzzle::move(std::vector<int_fast32_t> &moves, const int topTurns, const int bottomTurns) {
	move(topTurns, bottomTurns);
	moves.push_back(encodeMove(topTurns, bottomTurns));
}

void Puzzle::slice() {
	if (!canSlice()) {
		throw std::logic_error("Cannot perform a slice operation if a slice move is currently unavailable.");
	}

	/**
	 * HALF_MASK is a binary number with a 1 for the entire left half of the puzzle.
	 * 00000000000000000000 111111111111111111111111111111111111111111111111111111 000000000000000000000000000000000000000000000000000000
	 * And is used to isolate each half before swapping.
	 */
	const Row topHalf = top & HALF_MASK;
	const Row bottomHalf = bottom & HALF_MASK;

	top = top & ~HALF_MASK | bottomHalf;
	bottom = bottom & ~HALF_MASK | topHalf;
}

[[nodiscard]] bool Puzzle::cubeShape() const {
	/** TOP_CUBE_SHAPE and BOTTOM_CUBE_SHAPE are binary numbers with a 1 in each slot where an edge should be.
	* 00000000000000000000 000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001
	* 00000000000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001 000000000000 000001 000000000000
	* As there are exactly 12 edges, checking if exactly 12 slots are edges should never let an extra edge slip by.
	* And checking that they're in the right order, it is impossible for anything other than a corner to be in the remaning locations.
	* Assuming no illegal moves have been performed, this implies cube shape.
	* Small note; These need to be two different numbers as the bottom row is in reverse order.
	*/
	return (top & TOP_CUBE_SHAPE) == 0 && (bottom & BOTTOM_CUBE_SHAPE) == 0;
}

[[nodiscard]] bool Puzzle::canSlice() const {
	/**
	 * SLICE_MASK is a binary number with a 1 in the Corner Parity bit for the slots in location 0 and 8
	 * 00000000000000000000 010000 000000000000000000000000000000000000000000000000 010000 000000000000000000000000000000000000000000000000
	 * Assuming no illegal moves have been performed, the left and right halves of a corner will always stay together.
	 * Therefor, we can know if a corner is between the slice axis by checking if the right half of the corner is on the right of the slice
	 * which correspond to slots 0 and 8.
	 */
	return (top & SLICE_MASK) == 0 && (bottom & SLICE_MASK) == 0;
}

[[nodiscard]] bool Puzzle::canSliceTop() const {
	// See canSlice for details
	return (top & SLICE_MASK) == 0;
}

[[nodiscard]] bool Puzzle::canSliceBottom() const {
	// See canSlice for details
	return (bottom & SLICE_MASK) == 0;
}

[[nodiscard]] bool Puzzle::isRowOrientationSolved() const {
	/**
	 * ROW_ORIENTATION_MASK is a binary number with 1's in every Face Parity bit for each slot.
	 * 00000000000000000000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000 100000
	 * This checks if all of the face parity bits are zero, indicating they're top pieces
	 * Then does the same to the bottom by inverting it first, which turns what would be 1's in those bits to 0's.
	 * If there are any pieces in the wrong row, it will be picked up by this mask.
	 */
	return (top & ROW_ORIENTATION_MASK) == 0 && (~bottom & ROW_ORIENTATION_MASK) == 0;
}

[[nodiscard]] bool Puzzle::isSolvedByMatches(const Row topMatch, const Row topMask, const Row bottomMatch,
                                             const Row bottomMask) const {
	if (!cubeShape()) {
		return false;
	}

	if (!isRowOrientationSolved()) {
		return false;
	}

	return (top & topMask) == (topMatch & topMask) && (bottom & bottomMask) == (bottomMatch & bottomMask);
}

[[nodiscard]] bool Puzzle::isSolved() const {
	return top == SOLVED_TOP && bottom == SOLVED_BOTTOM;
}

[[nodiscard]] bool Puzzle::isTopSolved() const {
	return top == SOLVED_TOP;
}

[[nodiscard]] bool Puzzle::isBottomSolved() const {
	return bottom == SOLVED_BOTTOM;
}

[[nodiscard]] Puzzle Puzzle::clone() const {
	return {top, bottom};
}

void Puzzle::printRow(const Row row) {
	for (int i = 0; i < SLOTS_PER_ROW; ++i) {
		const Row slot = row >> ((SLOTS_PER_ROW - 1 - i) * SLOT_SIZE) & SLOT_MASK;
		std::cout << std::bitset<6>(slot) << ' ';
	}
}

void Puzzle::print() const {
	std::cout << "Top: ";
	printRow(top);
	std::cout << '\n';
	std::cout << "Bottom: ";
	printRow(bottom);
	std::cout << '\n';
	std::cout << "Cube Shape: " << (cubeShape() ? "true" : "false") << '\n';
	std::cout << "Can Slice: " << (canSlice() ? "true" : "false") << '\n';
	std::cout << "R.O. Solved: " << (isRowOrientationSolved() ? "true" : "false") << '\n';
	std::cout << "Is Solved: " << (isSolved() ? "true" : "false") << '\n';
}
