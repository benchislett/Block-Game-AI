#pragma once

#include "board.hpp"
#include "pieces.hpp"
#include <vector>
#include <array>
#include <random>
#include <functional>

namespace BlockGame {

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
    [[nodiscard]] bool canPlacePiece(int pieceType, int row, int col) const;

    // Generate all legal moves for a specific hand piece
    [[nodiscard]] std::vector<Move> getLegalMoves(int handIndex) const;
    
    // Generate all legal moves for all remaining pieces in hand
    [[nodiscard]] std::vector<Move> getAllLegalMoves() const;
    
    // Check if any moves are possible
    [[nodiscard]] bool hasLegalMoves() const;

    // Make a move, returns points scored from this move
    int makeMove(const Move& move, bool drawNewHand = true);
    
    // Place piece at position from hand index, returns points scored
    int placePiece(int handIndex, int row, int col, bool drawNewHand = true);

    // Start a new turn (draw new hand)
    void newTurn();

    // Reset game to initial state
    void reset();

    // Calculate score for clearing lines
    static int calculateClearScore(int linesCleared);

    // Update state after board modification (public helper for external solvers)
    // Updates score based on cleared lines and advances turn if needed
    // handIndex is required to mark the piece as used
    void commitMove(int handIndex, int linesCleared, bool drawNewHand = true);

// private:
    Board board_;
    std::array<int, HAND_SIZE> hand_;        // Piece types in hand
    std::array<bool, HAND_SIZE> handUsed_;   // Which pieces have been placed
    int score_;
    int turnNumber_;
    bool gameOver_;
    std::mt19937 rng_;

    void drawHand();
    void checkGameOver();
};

} // namespace BlockGame
