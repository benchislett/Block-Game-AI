#pragma once

#include "board.hpp"
#include "pieces.hpp"
#include <vector>
#include <array>
#include <random>
#include <functional>

namespace BlockGame {

/**
 * Represents a move: placing a piece at a position
 */
struct Move {
    int pieceIndex;  // Which piece from the current hand
    int row;
    int col;
    Board::Mask mask;  // Precomputed shifted mask

    bool operator==(const Move& other) const {
        return pieceIndex == other.pieceIndex && row == other.row && col == other.col;
    }
};

/**
 * Game state and logic
 */
class Game {
public:
    static constexpr int HAND_SIZE = 3;

    Game();
    explicit Game(uint64_t seed);

    // Game state accessors
    [[nodiscard]] const Board& board() const { return board_; }
    [[nodiscard]] int score() const { return score_; }
    [[nodiscard]] bool isGameOver() const { return gameOver_; }
    [[nodiscard]] const std::array<int, HAND_SIZE>& hand() const { return hand_; }
    [[nodiscard]] const std::array<bool, HAND_SIZE>& handUsed() const { return handUsed_; }
    [[nodiscard]] int turnNumber() const { return turnNumber_; }

    // Check if a specific piece from hand can be placed at position
    [[nodiscard]] bool canPlace(int handIndex, int row, int col) const;
    
    // Check if a piece (by piece type index) can be placed at position
    [[nodiscard]] bool canPlacePiece(int pieceIndex, int row, int col) const;

    // Generate all legal moves for a specific hand piece
    [[nodiscard]] std::vector<Move> getLegalMoves(int handIndex) const;
    
    // Generate all legal moves for all remaining pieces in hand
    [[nodiscard]] std::vector<Move> getAllLegalMoves() const;
    
    // Check if any moves are possible
    [[nodiscard]] bool hasLegalMoves() const;

    // Make a move, returns points scored from this move
    int makeMove(const Move& move);
    
    // Place piece at position, returns points scored
    int placePiece(int handIndex, int row, int col);

    // Start a new turn (draw new hand)
    void newTurn();

    // Reset game to initial state
    void reset();

    // Calculate score for clearing lines
    static int calculateClearScore(int linesCleared);

private:
    Board board_;
    std::array<int, HAND_SIZE> hand_;        // Piece indices in hand
    std::array<bool, HAND_SIZE> handUsed_;   // Which pieces have been placed
    int score_;
    int turnNumber_;
    bool gameOver_;
    std::mt19937 rng_;

    void drawHand();
    void checkGameOver();
};

/**
 * Efficient move generation utilities
 */
namespace MoveGen {

// Generate all valid positions for a piece on a board
// Calls callback with (row, col, mask) for each valid position
template<typename Callback>
void forEachValidPlacement(const Board& board, const Piece& piece, Callback&& callback) {
    const int maxRow = 8 - piece.height;
    const int maxCol = 8 - piece.width;
    
    for (int row = 0; row <= maxRow; ++row) {
        for (int col = 0; col <= maxCol; ++col) {
            Board::Mask mask = piece.shiftTo(row, col);
            if (board.canPlace(mask)) {
                callback(row, col, mask);
            }
        }
    }
}

// Count valid placements for a piece
int countValidPlacements(const Board& board, const Piece& piece);

// Check if any valid placement exists for a piece
bool hasValidPlacement(const Board& board, const Piece& piece);

} // namespace MoveGen

} // namespace BlockGame
