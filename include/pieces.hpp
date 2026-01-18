#pragma once

#include "board.hpp"
#include <vector>
#include <string>
#include <array>

namespace BlockGame {

// Forward declarations
class Board;

// Precomputed shifted masks for a piece type at each board position
// Index: row * 8 + col (0-63 for 8x8 board)
// Value: shifted mask, or 0 if out of bounds
struct PieceShiftTable {
    std::array<Board::Mask, 64> masks;  // Precomputed shifted masks for each position
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
    Board::Mask baseMask;   // Piece shape at origin (0,0)
    int width;              // Bounding box width
    int height;             // Bounding box height
    std::string name;       // Display name
    PieceShiftTable shiftTable;  // Precomputed shifted masks

    // Shift piece mask to a board position (row, col)
    // Returns 0 if piece would be out of bounds
    [[nodiscard]] inline Board::Mask shiftTo(int row, int col) const {
        // Use unsigned comparison to check both < 0 and > max in one check
        if (static_cast<unsigned>(row) > static_cast<unsigned>(shiftTable.maxRow) ||
            static_cast<unsigned>(col) > static_cast<unsigned>(shiftTable.maxCol)) {
            return 0;
        }
        return shiftTable.masks[row * 8 + col];
    }

    // Get string representation of piece shape
    [[nodiscard]] std::string toString() const;
};

// Piece index constants for reference
enum PieceType {
    // Squares
    SQUARE_2X2 = 0,
    SQUARE_3X3,
    
    // Rectangles
    RECT_2X3,
    RECT_3X2,
    
    // Lines
    LINE_3X1,
    LINE_4X1,
    LINE_5X1,
    LINE_1X3,
    LINE_1X4,
    LINE_1X5,
    
    // S pieces (4 rotations)
    S_0, S_90, S_180, S_270,
    
    // T pieces (4 rotations)
    T_0, T_90, T_180, T_270,
    
    // Small corners (2x2 minus corner, 4 rotations)
    SMALL_CORNER_0, SMALL_CORNER_90, SMALL_CORNER_180, SMALL_CORNER_270,
    
    // Large corners (3x3 minus 2x2, 4 rotations)
    LARGE_CORNER_0, LARGE_CORNER_90, LARGE_CORNER_180, LARGE_CORNER_270,
    
    // L pieces (4 rotations)
    L_0, L_90, L_180, L_270,
    
    // J pieces (mirrored L, 4 rotations)
    J_0, J_90, J_180, J_270,
    
    PIECE_COUNT
};

// Get all pieces
const std::vector<Piece>& getAllPieces();

// Get piece by index
const Piece& getPiece(int index);

// Number of pieces
constexpr int NUM_PIECES = static_cast<int>(PIECE_COUNT);

} // namespace BlockGame
