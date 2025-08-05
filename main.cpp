#include <bitset>
#include <cstdint>
#include <iostream>
#include <vector>
#include <future>
#include <sstream>
#include <utility>
#include <chrono>
#include "Puzzle.h"

static constexpr int_fast32_t MOVES[] = {0, 3, 15, 6, 12, 9, 1, 17, 2};
static constexpr int SIZE_OF_MOVES = std::size(MOVES);

std::mutex mutexLock;

std::string formatMoves(const std::vector<int_fast32_t> &moves, const bool endsOnSlice) {
	std::vector<int_fast32_t> ignoreZeroes = {};

	int sliceParity = 0;
	for (int i = 0; i <= moves.size(); ++i) {
		if (i == moves.size()) {
			if (sliceParity % 2 == 1) {
				ignoreZeroes.push_back(moves[i - 1]);
			}
			break;
		}
		if (moves[i] == 0) {
			sliceParity++;
			continue;
		}
		if (sliceParity % 2 == 1) {
			ignoreZeroes.push_back(moves[i - 1]);
		}
		ignoreZeroes.push_back(moves[i]);
		sliceParity = 0;
	}

	std::vector<int_fast32_t> previousList = ignoreZeroes;
	bool madeNoChange = false;
	while (!madeNoChange) {
		madeNoChange = true;
		int_fast32_t lastMove = 0;
		std::vector<int_fast32_t> ignoreDuplicates = {};
		for (int i = 0; i < previousList.size(); ++i) {
			if (previousList[i] == 0) {
				continue;
			}

			auto [lastTopTurns, lastBottomTurns] = Puzzle::decodeMove(lastMove);
			auto [topTurns, bottomTurns] = Puzzle::decodeMove(previousList[i]);
			if (
				previousList[i] != 0 &&
				lastTopTurns == Puzzle::wrapPositive(-topTurns) &&
				lastBottomTurns == Puzzle::wrapPositive(-bottomTurns)) {
				ignoreDuplicates.pop_back();
				lastMove = 0;
				madeNoChange = false;
				continue;
			}
			ignoreDuplicates.push_back(previousList[i]);
			lastMove = previousList[i];
		}
		previousList = ignoreDuplicates;
	}

	std::ostringstream out;
	std::cout << "Solution found in " << previousList.size() << " moves:\n";

	for (int i = 0; i < previousList.size(); ++i) {
		const int_fast32_t move = previousList[i];
		if (move != 0) {
			auto [topTurns, bottomTurns] = Puzzle::decodeMove(move);
			out << Puzzle::wrapNegative(topTurns) << " " << Puzzle::wrapNegative(bottomTurns) << " ";
		}

		if ((i < previousList.size() - (endsOnSlice ? 0 : 1)) && !(move == 0 && i > 0)) {
			out << "/ ";
		}
	}

	return out.str();
}

void checkSolved(const Puzzle &puzzle, const std::vector<int_fast32_t> &moves, const bool endsOnSlice) {
	if (puzzle.cubeShape() && puzzle.isRowOrientationSolved()) {
		std::lock_guard lock(mutexLock);
		std::cout << formatMoves(moves, endsOnSlice);
		std::exit(0);
	}
}

void solve(const Puzzle &puzzle, const std::vector<int_fast32_t> &moves, const int depth) {
	if (depth > 8) {
		return;
	}

	for (int_fast32_t a = 0; a < SIZE_OF_MOVES; ++a) {
		Puzzle topNext = puzzle.clone();
		topNext.turn(MOVES[a], 0);

		if (!topNext.canSliceTop()) {
			continue;
		}

		for (int_fast32_t b = 0; b < SIZE_OF_MOVES; ++b) {
			Puzzle bottomNext = topNext.clone();
			bottomNext.turn(0, MOVES[b]);

			if (!bottomNext.canSliceBottom()) {
				continue;
			}

			auto nextMoves = moves;
			nextMoves.push_back(Puzzle::encodeMove(MOVES[a], MOVES[b]));
			checkSolved(bottomNext, nextMoves, false);
			bottomNext.slice();
			checkSolved(bottomNext, nextMoves, true);
			solve(bottomNext, nextMoves, depth + 1);
		}
	}
}

void solveMultithread(const Puzzle &start, const std::vector<int_fast32_t> &baseMoves) {
	std::vector<std::future<void> > futures;

	for (int_fast32_t a = 0; a < SIZE_OF_MOVES; ++a) {
		Puzzle topNext = start.clone();
		topNext.turn(MOVES[a], 0);

		if (!topNext.canSliceTop()) {
			continue;
		}

		futures.emplace_back(std::async(std::launch::async, [topNext, a, baseMoves]() {
			for (int_fast32_t b = 0; b < SIZE_OF_MOVES; ++b) {
				Puzzle bottomNext = topNext.clone();
				bottomNext.turn(0, MOVES[b]);

				if (!bottomNext.canSliceBottom()) {
					continue;
				}

				auto nextMoves = baseMoves;
				nextMoves.push_back(Puzzle::encodeMove(MOVES[a], MOVES[b]));
				checkSolved(bottomNext, nextMoves, false);
				bottomNext.slice();
				checkSolved(bottomNext, nextMoves, true);
				solve(bottomNext, nextMoves, 1);
			}
		}));
	}

	for (auto &fut: futures) {
		fut.get();
	}
}

int main() {
	Puzzle start;

	std::vector<int_fast32_t> baseMoves = {};
	start.move(baseMoves, 0, 0);
	start.move(baseMoves, 3, 0);
	start.move(baseMoves, -3, -3);
	start.move(baseMoves, 0, 3);
	start.move(baseMoves, 1, 0);
	start.move(baseMoves, 0, 0);
	start.move(baseMoves, 0, 0);
	start.move(baseMoves, 0, 0);
	start.move(baseMoves, 0, 0);
	start.move(baseMoves, 3, 0);
	start.move(baseMoves, -3, -3);
	start.move(baseMoves, 0, 3);

	solveMultithread(start, baseMoves);

	std::cout << "No solution found.\n";
	return 0;
}
