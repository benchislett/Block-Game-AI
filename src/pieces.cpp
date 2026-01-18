#include "pieces.hpp"
#include <sstream>

namespace BlockGame {

namespace {

// Helper to create a mask from a pattern string
// Pattern: rows separated by |, X = filled, . = empty
// Example: "XX|XX" creates 2x2 square
constexpr Board::Mask createMask(const char* pattern) {
    Board::Mask mask = 0;
    int row = 0, col = 0;
    for (const char* p = pattern; *p; ++p) {
        if (*p == '|') {
            row++;
            col = 0;
        } else if (*p == 'X') {
            mask |= 1ULL << (row * 8 + col);
            col++;
        } else if (*p == '.') {
            col++;
        }
    }
    return mask;
}

// Calculate width from pattern
constexpr int getWidth(const char* pattern) {
    int maxWidth = 0;
    int currentWidth = 0;
    for (const char* p = pattern; *p; ++p) {
        if (*p == '|') {
            if (currentWidth > maxWidth) maxWidth = currentWidth;
            currentWidth = 0;
        } else if (*p == 'X' || *p == '.') {
            currentWidth++;
        }
    }
    if (currentWidth > maxWidth) maxWidth = currentWidth;
    return maxWidth;
}

// Calculate height from pattern
constexpr int getHeight(const char* pattern) {
    int height = 1;
    for (const char* p = pattern; *p; ++p) {
        if (*p == '|') height++;
    }
    return height;
}

// Compute the shifted mask for a piece at a given position
inline Board::Mask computeShiftedMask(Board::Mask baseMask, int width, int height, int row, int col) {
    if (row < 0 || col < 0 || row + height > 8 || col + width > 8) {
        return 0;
    }
    Board::Mask shifted = 0;
    for (int r = 0; r < height; ++r) {
        Board::Mask rowBits = (baseMask >> (r * 8)) & 0xFF;
        shifted |= rowBits << ((row + r) * 8 + col);
    }
    return shifted;
}

// Build the precomputed shift table for a piece
PieceShiftTable buildShiftTable(Board::Mask baseMask, int width, int height) {
    PieceShiftTable table;
    table.maxRow = 8 - height;
    table.maxCol = 8 - width;
    
    for (int pos = 0; pos < 64; ++pos) {
        int row = pos / 8;
        int col = pos % 8;
        table.masks[pos] = computeShiftedMask(baseMask, width, height, row, col);
    }
    return table;
}

// Helper to create a complete Piece with precomputed shift table
Piece createPiece(const char* pattern, const char* name) {
    Board::Mask mask = createMask(pattern);
    int w = getWidth(pattern);
    int h = getHeight(pattern);
    return Piece{mask, w, h, name, buildShiftTable(mask, w, h)};
}

// All piece definitions
const std::vector<Piece> PIECES = {
    // Squares
    createPiece("XX|XX", "2x2 Square"),
    createPiece("XXX|XXX|XXX", "3x3 Square"),
    
    // Rectangles
    createPiece("XX|XX|XX", "2x3 Rectangle"),
    createPiece("XXX|XXX", "3x2 Rectangle"),
    
    // Horizontal lines
    createPiece("XXX", "3x1 Line"),
    createPiece("XXXX", "4x1 Line"),
    createPiece("XXXXX", "5x1 Line"),
    
    // Vertical lines
    createPiece("X|X|X", "1x3 Line"),
    createPiece("X|X|X|X", "1x4 Line"),
    createPiece("X|X|X|X|X", "1x5 Line"),
    
    // S piece rotations
    createPiece(".XX|XX.", "S piece 0°"),
    createPiece("X.|XX|.X", "S piece 90°"),
    createPiece("XX.|.XX", "S piece Mirrored"),
    createPiece(".X|XX|X.", "S piece 90° Mirrored"),
    
    // T piece rotations
    createPiece("XXX|.X.", "T piece 0°"),
    createPiece("X.|XX|X.", "T piece 90°"),
    createPiece(".X.|XXX", "T piece 180°"),
    createPiece(".X|XX|.X", "T piece 270°"),
    
    // Small corner (2x2 minus one corner) rotations
    createPiece("XX|X.", "Small Corner 0°"),
    createPiece("XX|.X", "Small Corner 90°"),
    createPiece(".X|XX", "Small Corner 180°"),
    createPiece("X.|XX", "Small Corner 270°"),
    
    // Large corner (3x3 minus 2x2 from corner) rotations
    createPiece("XXX|X..|X..", "Large Corner 0°"),
    createPiece("XXX|..X|..X", "Large Corner 90°"),
    createPiece("..X|..X|XXX", "Large Corner 180°"),
    createPiece("X..|X..|XXX", "Large Corner 270°"),
    
    // L piece rotations (Tetris L)
    createPiece("X.|X.|XX", "L piece 0°"),
    createPiece("XXX|X..", "L piece 90°"),
    createPiece("XX|.X|.X", "L piece 180°"),
    createPiece("..X|XXX", "L piece 270°"),
    
    // J piece rotations (mirrored L)
    createPiece(".X|.X|XX", "J piece 0°"),
    createPiece("X..|XXX", "J piece 90°"),
    createPiece("XX|X.|X.", "J piece 180°"),
    createPiece("XXX|..X", "J piece 270°"),
};

} // anonymous namespace

std::string Piece::toString() const {
    std::ostringstream oss;
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            if (baseMask & (1ULL << (r * 8 + c))) {
                oss << "█ ";
            } else {
                oss << "  ";
            }
        }
        if (r < height - 1) oss << "\n";
    }
    return oss.str();
}

const std::vector<Piece>& getAllPieces() {
    return PIECES;
}

const Piece& getPiece(int index) {
    return PIECES[index];
}

} // namespace BlockGame
