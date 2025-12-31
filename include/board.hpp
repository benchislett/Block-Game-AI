#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace BlockGame {

/**
 * 8x8 board represented as a single 64-bit value.
 * Bit layout: bit 0 = (0,0), bit 1 = (1,0), ..., bit 7 = (7,0), bit 8 = (0,1), etc.
 * Row i, Column j maps to bit (i * 8 + j)
 */
class Board {
public:
    using Mask = uint64_t;

    Board() : data_(0) {}
    explicit Board(Mask data) : data_(data) {}

    // Core bit operations
    [[nodiscard]] Mask data() const { return data_; }
    
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
    [[nodiscard]] bool canPlace(Mask pieceMask) const {
        return (data_ & pieceMask) == 0;
    }

    // Place a piece (OR operation)
    void place(Mask pieceMask) {
        data_ |= pieceMask;
    }

    // Check which rows and columns are full, returns masks
    [[nodiscard]] uint8_t getFullRows() const;
    [[nodiscard]] uint8_t getFullCols() const;
    
    // Clear full rows and columns, returns count of cleared lines
    int clearFullLines();

    // String representation for debugging/display
    [[nodiscard]] std::string toString() const;

    // Precomputed row and column masks
    static constexpr Mask ROW_MASK = 0xFF;  // First row
    static constexpr Mask COL_MASK = 0x0101010101010101ULL;  // First column
    static constexpr Mask FULL_BOARD = 0xFFFFFFFFFFFFFFFFULL;

    static constexpr Mask rowMask(int row) { return ROW_MASK << (row * 8); }
    static constexpr Mask colMask(int col) { return COL_MASK << col; }

private:
    Mask data_;

    static constexpr Mask bitAt(int row, int col) {
        return 1ULL << (row * 8 + col);
    }
};

} // namespace BlockGame
