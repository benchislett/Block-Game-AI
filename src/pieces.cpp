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

// All piece definitions
const std::vector<Piece> PIECES = {
    // Squares
    {createMask("XX|XX"), getWidth("XX|XX"), getHeight("XX|XX"), "2x2 Square"},
    {createMask("XXX|XXX|XXX"), getWidth("XXX|XXX|XXX"), getHeight("XXX|XXX|XXX"), "3x3 Square"},
    
    // Rectangles
    {createMask("XX|XX|XX"), getWidth("XX|XX|XX"), getHeight("XX|XX|XX"), "2x3 Rectangle"},
    {createMask("XXX|XXX"), getWidth("XXX|XXX"), getHeight("XXX|XXX"), "3x2 Rectangle"},
    
    // Horizontal lines
    {createMask("XXX"), getWidth("XXX"), getHeight("XXX"), "3x1 Line"},
    {createMask("XXXX"), getWidth("XXXX"), getHeight("XXXX"), "4x1 Line"},
    {createMask("XXXXX"), getWidth("XXXXX"), getHeight("XXXXX"), "5x1 Line"},
    
    // Vertical lines
    {createMask("X|X|X"), getWidth("X|X|X"), getHeight("X|X|X"), "1x3 Line"},
    {createMask("X|X|X|X"), getWidth("X|X|X|X"), getHeight("X|X|X|X"), "1x4 Line"},
    {createMask("X|X|X|X|X"), getWidth("X|X|X|X|X"), getHeight("X|X|X|X|X"), "1x5 Line"},
    
    // S piece rotations
    // S_0: .XX|XX.
    {createMask(".XX|XX."), getWidth(".XX|XX."), getHeight(".XX|XX."), "S piece 0°"},
    // S_90: X.|XX|.X
    {createMask("X.|XX|.X"), getWidth("X.|XX|.X"), getHeight("X.|XX|.X"), "S piece 90°"},
    // S_180: Mirrored of S_0
    {createMask("XX.|.XX"), getWidth("XX.|.XX"), getHeight("XX.|.XX"), "S piece Mirrored"},
    // S_270: Mirrored of S_90
    {createMask(".X|XX|X."), getWidth(".X|XX|X."), getHeight(".X|XX|X."), "S piece 90° Mirrored"},
    
    // T piece rotations
    // T_0: XXX|.X.
    {createMask("XXX|.X."), getWidth("XXX|.X."), getHeight("XXX|.X."), "T piece 0°"},
    // T_90: X.|XX|X.
    {createMask("X.|XX|X."), getWidth("X.|XX|X."), getHeight("X.|XX|X."), "T piece 90°"},
    // T_180: .X.|XXX
    {createMask(".X.|XXX"), getWidth(".X.|XXX"), getHeight(".X.|XXX"), "T piece 180°"},
    // T_270: .X|XX|.X
    {createMask(".X|XX|.X"), getWidth(".X|XX|.X"), getHeight(".X|XX|.X"), "T piece 270°"},
    
    // Small corner (2x2 minus one corner) rotations
    // SC_0: XX|X. (missing bottom-right)
    {createMask("XX|X."), getWidth("XX|X."), getHeight("XX|X."), "Small Corner 0°"},
    // SC_90: XX|.X (missing bottom-left)
    {createMask("XX|.X"), getWidth("XX|.X"), getHeight("XX|.X"), "Small Corner 90°"},
    // SC_180: .X|XX (missing top-left)
    {createMask(".X|XX"), getWidth(".X|XX"), getHeight(".X|XX"), "Small Corner 180°"},
    // SC_270: X.|XX (missing top-right)
    {createMask("X.|XX"), getWidth("X.|XX"), getHeight("X.|XX"), "Small Corner 270°"},
    
    // Large corner (3x3 minus 2x2 from corner) rotations
    // LC_0: XXX|X..|X.. (5 cells, L shape)
    {createMask("XXX|X..|X.."), getWidth("XXX|X..|X.."), getHeight("XXX|X..|X.."), "Large Corner 0°"},
    // LC_90: XXX|..X|..X
    {createMask("XXX|..X|..X"), getWidth("XXX|..X|..X"), getHeight("XXX|..X|..X"), "Large Corner 90°"},
    // LC_180: ..X|..X|XXX
    {createMask("..X|..X|XXX"), getWidth("..X|..X|XXX"), getHeight("..X|..X|XXX"), "Large Corner 180°"},
    // LC_270: X..|X..|XXX
    {createMask("X..|X..|XXX"), getWidth("X..|X..|XXX"), getHeight("X..|X..|XXX"), "Large Corner 270°"},
    
    // L piece rotations (Tetris L)
    // L_0: X..|X..|XX.
    {createMask("X.|X.|XX"), getWidth("X.|X.|XX"), getHeight("X.|X.|XX"), "L piece 0°"},
    // L_90: XXX|X..
    {createMask("XXX|X.."), getWidth("XXX|X.."), getHeight("XXX|X.."), "L piece 90°"},
    // L_180: XX|.X|.X
    {createMask("XX|.X|.X"), getWidth("XX|.X|.X"), getHeight("XX|.X|.X"), "L piece 180°"},
    // L_270: ..X|XXX
    {createMask("..X|XXX"), getWidth("..X|XXX"), getHeight("..X|XXX"), "L piece 270°"},
    
    // J piece rotations (mirrored L)
    // J_0: .X|.X|XX
    {createMask(".X|.X|XX"), getWidth(".X|.X|XX"), getHeight(".X|.X|XX"), "J piece 0°"},
    // J_90: X..|XXX
    {createMask("X..|XXX"), getWidth("X..|XXX"), getHeight("X..|XXX"), "J piece 90°"},
    // J_180: XX|X.|X.
    {createMask("XX|X.|X."), getWidth("XX|X.|X."), getHeight("XX|X.|X."), "J piece 180°"},
    // J_270: XXX|..X
    {createMask("XXX|..X"), getWidth("XXX|..X"), getHeight("XXX|..X"), "J piece 270°"},
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
