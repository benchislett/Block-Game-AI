#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include "types.hpp"

namespace BlockGame {

/**
 * Represents a move: placing a piece type at a position
 */
struct Move {
    PieceType type;   // The kind of piece (e.g., SQUARE_2X2)
    int row;
    int col;
    uint64_t mask;  // Precomputed shifted mask

    bool operator==(const Move& other) const {
        return type == other.type && row == other.row && col == other.col;
    }
};

/**
 * 8x8 board represented as a single 64-bit value.
 * Bit layout: bit 0 = (0,0), bit 1 = (1,0), ..., bit 7 = (7,0), bit 8 = (0,1), etc.
 * Row i, Column j maps to bit (i * 8 + j)
 */
class Board {
public:
    Board() : data_(0) {}
    explicit Board(uint64_t data) : data_(data) {}

    // Core bit operations
    [[nodiscard]] uint64_t data() const { return data_; }
    
    [[nodiscard]] bool isEmpty() const { return data_ == 0; }
    [[nodiscard]] bool isFull() const { return data_ == FULL_BOARD; }
    
    [[nodiscard]] bool isOccupied(int row, int col) const {
        return (data_ & bitAt(row, col)) != 0;
    }
    
    void setOccupied(int row, int col) {
        data_ |= bitAt(row, col);
    }
    
    void clearSquare(int row, int col) {
        data_ &= ~bitAt(row, col);
    }

    // Check if placing a piece (represented as mask shifted to position) is valid
    [[nodiscard]] bool canPlace(uint64_t pieceMask) const {
        return (data_ & pieceMask) == 0;
    }
    
    // High-level move generation and validation methods
    // These look up the Piece by type internally
    [[nodiscard]] bool canPlacePiece(PieceType type, int row, int col) const;
    [[nodiscard]] std::vector<Move> getLegalMoves(PieceType type) const;
    [[nodiscard]] std::vector<Move> getLegalMoves(const Piece& piece) const;
    [[nodiscard]] int countValidPlacements(PieceType type) const;

    // Place a piece (OR operation), does NOT clear lines
    void place(uint64_t pieceMask) {
        data_ |= pieceMask;
    }

    // Place a piece and clear full lines, returns number of lines cleared
    int placeAndClear(uint64_t pieceMask);
    
    // Clear full rows and columns, returns count of cleared lines
    int clearFullLines();

    // String representation for debugging/display
    [[nodiscard]] std::string toString() const;

    // Precomputed row and column masks
    static constexpr uint64_t ROW_MASK = 0xFF;  // First row
    static constexpr uint64_t COL_MASK = 0x0101010101010101ULL;  // First column
    static constexpr uint64_t FULL_BOARD = 0xFFFFFFFFFFFFFFFFULL;

    static constexpr uint64_t rowMask(int row) { return ROW_MASK << (row * 8); }
    static constexpr uint64_t colMask(int col) { return COL_MASK << col; }

private:
    uint64_t data_;

    static constexpr uint64_t bitAt(int row, int col) {
        return 1ULL << (row * 8 + col);
    }
};

} // namespace BlockGame
