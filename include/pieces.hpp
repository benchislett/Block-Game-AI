#pragma once

#include "board.hpp"
#include <vector>
#include <string>
#include <array>

namespace BlockGame {

// Forward declarations
class Board;

/**
 * A piece is defined by:
 * - A base bitmask (origin at top-left of piece bounding box at position 0,0)
 * - Width and height (bounding box dimensions)
 * - Name for display
 */
struct Piece {
    Board::Mask baseMask;   // Piece shape at origin (0,0)
    int width;              // Bounding box width
    int height;             // Bounding box height
    std::string name;       // Display name

    // Shift piece mask to a board position (row, col)
    // Returns 0 if piece would be out of bounds
    [[nodiscard]] Board::Mask shiftTo(int row, int col) const {
        if (row < 0 || col < 0 || row + height > 8 || col + width > 8) {
            return 0;
        }
        // Base mask has pieces at rows 0..height-1, cols 0..width-1
        // We need to shift to row, col
        // Row shift: multiply by 8 for each row
        // Col shift: add col bits
        Board::Mask shifted = 0;
        for (int r = 0; r < height; ++r) {
            // Extract row r of the base mask
            Board::Mask rowBits = (baseMask >> (r * 8)) & 0xFF;
            // Shift to target position
            shifted |= rowBits << ((row + r) * 8 + col);
        }
        return shifted;
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
