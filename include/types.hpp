#pragma once

#include <cstdint>

namespace BlockGame {

// Forward declarations
class Board;
struct Piece;

// Piece index constants
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

// Number of pieces
constexpr int NUM_PIECES = static_cast<int>(PIECE_COUNT);

} // namespace BlockGame
