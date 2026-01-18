#pragma once

#include "types.hpp"
#include <vector>
#include <string>
#include <array>
#include <cstdint>

namespace BlockGame {

// Precomputed shifted masks for a piece type at each board position
// Index: row * 8 + col (0-63 for 8x8 board)
// Value: shifted mask, or 0 if out of bounds
struct PieceShiftTable {
    std::array<uint64_t, 64> masks;  // Precomputed shifted masks for each position
    int maxRow;                          // Maximum valid row (8 - height)
    int maxCol;                          // Maximum valid col (8 - width)
};

/**
 * A piece is defined by:
 * - A base bitmask (origin at top-left of piece bounding box at position 0,0)
 * - Width and height (bounding box dimensions)
 * - Name for display
 * - Precomputed shift table for fast placement
 */
struct Piece {
    uint64_t baseMask;   // Piece shape at origin (0,0)
    int width;              // Bounding box width
    int height;             // Bounding box height
    std::string name;       // Display name
    PieceShiftTable shiftTable;  // Precomputed shifted masks
    PieceType type;         // Piece type index

    // Shift piece mask to a board position (row, col)
    // Returns 0 if piece would be out of bounds
    [[nodiscard]] inline uint64_t shiftTo(int row, int col) const {
        // Use unsigned comparison to check both < 0 and > max in one check
        if (static_cast<unsigned>(row) > static_cast<unsigned>(shiftTable.maxRow) ||
            static_cast<unsigned>(col) > static_cast<unsigned>(shiftTable.maxCol)) {
            return 0;
        }
        return shiftToUnsafe(row, col);
    }

    [[nodiscard]] inline uint64_t shiftToUnsafe(int row, int col) const {
        return shiftTable.masks[row * 8 + col];
    }

    // Get string representation of piece shape
    [[nodiscard]] std::string toString() const;
};

// Get all pieces
const std::vector<Piece>& getAllPieces();

// Get piece by index
const Piece& getPiece(PieceType type);

} // namespace BlockGame
