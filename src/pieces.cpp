#include "pieces.hpp"
#include <sstream>

namespace BlockGame {

namespace {

// Helper to create a mask from a pattern string
// Pattern: rows separated by |, X = filled, . = empty
// Example: "XX|XX" creates 2x2 square
constexpr uint64_t createMask(const char* pattern) {
    uint64_t mask = 0;
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
inline uint64_t computeShiftedMask(uint64_t baseMask, int width, int height, int row, int col) {
    if (row < 0 || col < 0 || row + height > 8 || col + width > 8) {
        return 0;
    }
    uint64_t shifted = 0;
    for (int r = 0; r < height; ++r) {
        uint64_t rowBits = (baseMask >> (r * 8)) & 0xFF;
        shifted |= rowBits << ((row + r) * 8 + col);
    }
    return shifted;
}

// Build the precomputed shift table for a piece
PieceShiftTable buildShiftTable(uint64_t baseMask, int width, int height) {
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
Piece createPiece(PieceType type, const char* pattern, const char* name) {
    uint64_t mask = createMask(pattern);
    int w = getWidth(pattern);
    int h = getHeight(pattern);
    return Piece{mask, w, h, name, buildShiftTable(mask, w, h), type};
}

// All piece definitions
const std::vector<Piece> PIECES = {
    // Squares
    createPiece(SQUARE_2X2, "XX|XX", "2x2 Square"),
    createPiece(SQUARE_3X3, "XXX|XXX|XXX", "3x3 Square"),
    
    // Rectangles
    createPiece(RECT_2X3, "XX|XX|XX", "2x3 Rectangle"),
    createPiece(RECT_3X2, "XXX|XXX", "3x2 Rectangle"),
    
    // Horizontal lines
    createPiece(LINE_3X1, "XXX", "3x1 Line"),
    createPiece(LINE_4X1, "XXXX", "4x1 Line"),
    createPiece(LINE_5X1, "XXXXX", "5x1 Line"),
    
    // Vertical lines
    createPiece(LINE_1X3, "X|X|X", "1x3 Line"),
    createPiece(LINE_1X4, "X|X|X|X", "1x4 Line"),
    createPiece(LINE_1X5, "X|X|X|X|X", "1x5 Line"),
    
    // S piece rotations
    createPiece(S_0, ".XX|XX.", "S piece 0°"),
    createPiece(S_90, "X.|XX|.X", "S piece 90°"),
    createPiece(S_180, "XX.|.XX", "S piece Mirrored"),
    createPiece(S_270, ".X|XX|X.", "S piece 90° Mirrored"),
    
    // T piece rotations
    createPiece(T_0, "XXX|.X.", "T piece 0°"),
    createPiece(T_90, "X.|XX|X.", "T piece 90°"),
    createPiece(T_180, ".X.|XXX", "T piece 180°"),
    createPiece(T_270, ".X|XX|.X", "T piece 270°"),
    
    // Small corner (2x2 minus one corner) rotations
    createPiece(SMALL_CORNER_0, "XX|X.", "Small Corner 0°"),
    createPiece(SMALL_CORNER_90, "XX|.X", "Small Corner 90°"),
    createPiece(SMALL_CORNER_180, ".X|XX", "Small Corner 180°"),
    createPiece(SMALL_CORNER_270, "X.|XX", "Small Corner 270°"),
    
    // Large corner (3x3 minus 2x2 from corner) rotations
    createPiece(LARGE_CORNER_0, "XXX|X..|X..", "Large Corner 0°"),
    createPiece(LARGE_CORNER_90, "XXX|..X|..X", "Large Corner 90°"),
    createPiece(LARGE_CORNER_180, "..X|..X|XXX", "Large Corner 180°"),
    createPiece(LARGE_CORNER_270, "X..|X..|XXX", "Large Corner 270°"),
    
    // L piece rotations (Tetris L)
    createPiece(L_0, "X.|X.|XX", "L piece 0°"),
    createPiece(L_90, "XXX|X..", "L piece 90°"),
    createPiece(L_180, "XX|.X|.X", "L piece 180°"),
    createPiece(L_270, "..X|XXX", "L piece 270°"),
    
    // J piece rotations (mirrored L)
    createPiece(J_0, ".X|.X|XX", "J piece 0°"),
    createPiece(J_90, "X..|XXX", "J piece 90°"),
    createPiece(J_180, "XX|X.|X.", "J piece 180°"),
    createPiece(J_270, "XXX|..X", "J piece 270°"),
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

const Piece& getPiece(PieceType type) {
    return PIECES[type];
}

} // namespace BlockGame
