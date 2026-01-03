#include "board.hpp"
#include "pieces.hpp"
#include <bitset>
#include <sstream>

namespace BlockGame {

uint8_t Board::getFullRows() const {
    uint8_t fullRows = 0;
    for (int r = 0; r < 8; ++r) {
        if ((data_ & rowMask(r)) == rowMask(r)) {
            fullRows |= (1 << r);
        }
    }
    return fullRows;
}

uint8_t Board::getFullCols() const {
    uint8_t fullCols = 0;
    for (int c = 0; c < 8; ++c) {
        if ((data_ & colMask(c)) == colMask(c)) {
            fullCols |= (1 << c);
        }
    }
    return fullCols;
}

int Board::clearFullLines() {
    uint8_t fullRows = getFullRows();
    uint8_t fullCols = getFullCols();
    
    if (fullRows == 0 && fullCols == 0) {
        return 0;
    }
    
    int linesCleared = 0;
    Mask keepMask = FULL_BOARD;
    
    // Clear rows
    for (int r = 0; r < 8; ++r) {
        if (fullRows & (1 << r)) {
            keepMask &= ~rowMask(r);
            linesCleared++;
        }
    }
    
    // Clear cols
    for (int c = 0; c < 8; ++c) {
        if (fullCols & (1 << c)) {
            keepMask &= ~colMask(c);
            linesCleared++;
        }
    }
    
    data_ &= keepMask;
    return linesCleared;
}

int Board::placeAndClear(Mask pieceMask) {
    place(pieceMask);
    return clearFullLines();
}

bool Board::canPlacePiece(int pieceType, int row, int col) const {
    const Piece& piece = getPiece(pieceType);
    Board::Mask mask = piece.shiftTo(row, col);
    if (mask == 0) return false;  // Out of bounds
    return canPlace(mask);
}

std::vector<Move> Board::getLegalMoves(int pieceType) const {
    std::vector<Move> moves;
    const Piece& piece = getPiece(pieceType);
    
    const int maxRow = 8 - piece.height;
    const int maxCol = 8 - piece.width;
    
    for (int row = 0; row <= maxRow; ++row) {
        for (int col = 0; col <= maxCol; ++col) {
            Board::Mask mask = piece.shiftTo(row, col);
            if (canPlace(mask)) {
                moves.push_back({pieceType, row, col, mask});
            }
        }
    }
    return moves;
}

int Board::countValidPlacements(int pieceType) const {
    int count = 0;
    const Piece& piece = getPiece(pieceType);
    
    const int maxRow = 8 - piece.height;
    const int maxCol = 8 - piece.width;
    
    for (int row = 0; row <= maxRow; ++row) {
        for (int col = 0; col <= maxCol; ++col) {
            Board::Mask mask = piece.shiftTo(row, col);
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
