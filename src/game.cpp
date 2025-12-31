#include "game.hpp"
#include <chrono>
#include <algorithm>

namespace BlockGame {

Game::Game() 
    : Game(std::chrono::steady_clock::now().time_since_epoch().count()) {
}

Game::Game(uint64_t seed) 
    : board_()
    , hand_{}
    , handUsed_{}
    , score_(0)
    , turnNumber_(0)
    , gameOver_(false)
    , rng_(seed) {
    drawHand();
}

void Game::reset() {
    board_ = Board();
    score_ = 0;
    turnNumber_ = 0;
    gameOver_ = false;
    std::fill(handUsed_.begin(), handUsed_.end(), false);
    drawHand();
}

void Game::drawHand() {
    std::uniform_int_distribution<int> dist(0, NUM_PIECES - 1);
    for (int i = 0; i < HAND_SIZE; ++i) {
        hand_[i] = dist(rng_);
        handUsed_[i] = false;
    }
    turnNumber_++;
    checkGameOver();
}

bool Game::canPlace(int handIndex, int row, int col) const {
    if (handIndex < 0 || handIndex >= HAND_SIZE || handUsed_[handIndex]) {
        return false;
    }
    return canPlacePiece(hand_[handIndex], row, col);
}

bool Game::canPlacePiece(int pieceIndex, int row, int col) const {
    const Piece& piece = getPiece(pieceIndex);
    Board::Mask mask = piece.shiftTo(row, col);
    if (mask == 0) return false;  // Out of bounds
    return board_.canPlace(mask);
}

std::vector<Move> Game::getLegalMoves(int handIndex) const {
    std::vector<Move> moves;
    if (handIndex < 0 || handIndex >= HAND_SIZE || handUsed_[handIndex]) {
        return moves;
    }

    const Piece& piece = getPiece(hand_[handIndex]);
    MoveGen::forEachValidPlacement(board_, piece, [&](int row, int col, Board::Mask mask) {
        moves.push_back({handIndex, row, col, mask});
    });
    
    return moves;
}

std::vector<Move> Game::getAllLegalMoves() const {
    std::vector<Move> allMoves;
    for (int i = 0; i < HAND_SIZE; ++i) {
        if (!handUsed_[i]) {
            auto moves = getLegalMoves(i);
            allMoves.insert(allMoves.end(), moves.begin(), moves.end());
        }
    }
    return allMoves;
}

bool Game::hasLegalMoves() const {
    for (int i = 0; i < HAND_SIZE; ++i) {
        if (!handUsed_[i]) {
            const Piece& piece = getPiece(hand_[i]);
            if (MoveGen::hasValidPlacement(board_, piece)) {
                return true;
            }
        }
    }
    return false;
}

int Game::makeMove(const Move& move) {
    if (gameOver_ || move.pieceIndex < 0 || move.pieceIndex >= HAND_SIZE || 
        handUsed_[move.pieceIndex]) {
        return 0;
    }
    
    return placePiece(move.pieceIndex, move.row, move.col);
}

int Game::placePiece(int handIndex, int row, int col) {
    if (!canPlace(handIndex, row, col)) {
        return 0;
    }

    const Piece& piece = getPiece(hand_[handIndex]);
    Board::Mask mask = piece.shiftTo(row, col);
    
    // Place the piece
    board_.place(mask);
    handUsed_[handIndex] = true;
    
    // Clear full lines and calculate score
    int linesCleared = board_.clearFullLines();
    int pointsEarned = calculateClearScore(linesCleared);
    score_ += pointsEarned;
    
    // Check if all pieces placed or game over
    bool allPlaced = std::all_of(handUsed_.begin(), handUsed_.end(), [](bool b) { return b; });
    if (allPlaced) {
        drawHand();
    } else {
        checkGameOver();
    }
    
    return pointsEarned;
}

void Game::newTurn() {
    drawHand();
}

void Game::checkGameOver() {
    if (!hasLegalMoves()) {
        gameOver_ = true;
    }
}

int Game::calculateClearScore(int linesCleared) {
    return linesCleared * linesCleared * 8;
}

namespace MoveGen {

int countValidPlacements(const Board& board, const Piece& piece) {
    int count = 0;
    forEachValidPlacement(board, piece, [&](int, int, Board::Mask) {
        count++;
    });
    return count;
}

bool hasValidPlacement(const Board& board, const Piece& piece) {
    const int maxRow = 8 - piece.height;
    const int maxCol = 8 - piece.width;
    
    for (int row = 0; row <= maxRow; ++row) {
        for (int col = 0; col <= maxCol; ++col) {
            Board::Mask mask = piece.shiftTo(row, col);
            if (board.canPlace(mask)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace MoveGen

} // namespace BlockGame
