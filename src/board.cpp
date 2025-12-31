#include "board.hpp"
#include <sstream>

namespace BlockGame {

uint8_t Board::getFullRows() const {
    uint8_t fullRows = 0;
    for (int row = 0; row < 8; ++row) {
        if ((data_ & rowMask(row)) == rowMask(row)) {
            fullRows |= (1 << row);
        }
    }
    return fullRows;
}

uint8_t Board::getFullCols() const {
    uint8_t fullCols = 0;
    for (int col = 0; col < 8; ++col) {
        if ((data_ & colMask(col)) == colMask(col)) {
            fullCols |= (1 << col);
        }
    }
    return fullCols;
}

int Board::clearFullLines() {
    uint8_t fullRows = getFullRows();
    uint8_t fullCols = getFullCols();
    
    // Clear full rows
    for (int row = 0; row < 8; ++row) {
        if (fullRows & (1 << row)) {
            data_ &= ~rowMask(row);
        }
    }
    
    // Clear full columns
    for (int col = 0; col < 8; ++col) {
        if (fullCols & (1 << col)) {
            data_ &= ~colMask(col);
        }
    }
    
    return __builtin_popcount(fullRows) + __builtin_popcount(fullCols);
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
