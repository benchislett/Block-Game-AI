#include "board.hpp"
#include "pieces.hpp"
#include <bitset>
#include <sstream>
#include <stdint.h>

namespace BlockGame {

int Board::clearFullLines() {
    uint64_t c = data_;
    c &= (c >> 8);
    c &= (c >> 16);
    c &= (c >> 32);
    uint64_t col_ind = c & 0xFF;

    // 2. Row Reduction (Horizontal)
    // Can execute in parallel with step 1 due to Out-of-Order execution
    uint64_t r = data_;
    r &= (r >> 1);
    r &= (r >> 2);
    r &= (r >> 4);
    // 0x0101010101010101 allows us to mask the specific bits (0, 8, 16...) 
    // AND broadcast the column bits later. We load it once.
    const uint64_t kBroadcast = 0x0101010101010101ULL;
    uint64_t row_ind = r & kBroadcast;

    // 3. Mask Generation (The Optimization)
    // We combine the masks using OR before applying them to the grid.
    // We force a multiply for the row_mask to use the Multiplier Port (Port 1)
    // which is likely idle, relieving pressure on the Shift/Add ports.
    
    // col_mask: 00000001 -> 01010101
    uint64_t col_mask = col_ind * kBroadcast; 
    
    // row_mask: 0x...0100... -> 0x...FF00...
    // We use explicit multiplication by 255.
    uint64_t row_mask = row_ind * 255; 

    // Combine masks to break serial dependency on 'grid'
    uint64_t total_mask = col_mask | row_mask;

    data_ &= ~total_mask;
    return __builtin_popcountll(col_ind) + __builtin_popcountll(row_ind);
}

int Board::placeAndClear(uint64_t pieceMask) {
    place(pieceMask);
    return clearFullLines();
}

bool Board::canPlacePiece(PieceType type, int row, int col) const {
    const Piece& piece = getPiece(type);
    uint64_t mask = piece.shiftTo(row, col);
    if (mask == 0) return false;  // Out of bounds
    return canPlace(mask);
}

std::vector<Move> Board::getLegalMoves(PieceType type) const {
    return getLegalMoves(getPiece(type));
}

std::vector<Move> Board::getLegalMoves(const Piece& piece) const {
    std::vector<Move> moves;
    
    for (int row = 0; row <= piece.shiftTable.maxRow; ++row) {
        for (int col = 0; col <= piece.shiftTable.maxCol; ++col) {
            uint64_t mask = piece.shiftToUnsafe(row, col);
            if (canPlace(mask)) {
                moves.push_back({piece.type, row, col, mask});
            }
        }
    }
    return moves;
}

int Board::countValidPlacements(PieceType type) const {
    int count = 0;
    const Piece& piece = getPiece(type);
    
    const int maxRow = 8 - piece.height;
    const int maxCol = 8 - piece.width;
    
    for (int row = 0; row <= maxRow; ++row) {
        for (int col = 0; col <= maxCol; ++col) {
            uint64_t mask = piece.shiftTo(row, col);
            if (canPlace(mask)) {
                count++;
            }
        }
    }
    return count;
}

std::string Board::toString() const {
    std::ostringstream oss;
    oss << "  0 1 2 3 4 5 6 7\n";
    for (int row = 0; row < 8; ++row) {
        oss << row << " ";
        for (int col = 0; col < 8; ++col) {
            oss << (isOccupied(row, col) ? "█ " : "· ");
        }
        oss << "\n";
    }
    return oss.str();
}

} // namespace BlockGame
