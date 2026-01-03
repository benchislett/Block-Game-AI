#include "game.hpp"
#include <chrono>
#include <algorithm>
#include <iostream>

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
    return board_.canPlacePiece(hand_[handIndex], row, col);
}

bool Game::canPlacePiece(int pieceType, int row, int col) const {
    return board_.canPlacePiece(pieceType, row, col);
}

std::vector<Move> Game::getLegalMoves(int handIndex) const {
    if (handIndex < 0 || handIndex >= HAND_SIZE || handUsed_[handIndex]) {
        return {};
    }
    return board_.getLegalMoves(hand_[handIndex]);
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
            if (board_.countValidPlacements(hand_[i]) > 0) {
                return true;
            }
        }
    }
    return false;
}

int Game::makeMove(const Move& move, bool drawNewHand) {
    if (gameOver_) {
        return 0;
    }

    // Find the hand index for this move
    int handIndex = -1;
    for (int i = 0; i < HAND_SIZE; ++i) {
        if (!handUsed_[i] && hand_[i] == move.pieceType) {
            handIndex = i;
            break;
        }
    }

    if (handIndex == -1) {
        // This move is not valid for the current hand
        return 0;
    }
    
    return placePiece(handIndex, move.row, move.col, drawNewHand);
}

int Game::placePiece(int handIndex, int row, int col, bool drawNewHand) {
    if (!canPlace(handIndex, row, col)) {
        return 0;
    }

    const Piece& piece = getPiece(hand_[handIndex]);
    Board::Mask mask = piece.shiftTo(row, col);
    
    // Delegate to board for modification and line clearing
    int linesCleared = board_.placeAndClear(mask);
    
    // Update game state
    commitMove(handIndex, linesCleared, drawNewHand);
    
    return calculateClearScore(linesCleared);
}

void Game::commitMove(int handIndex, int linesCleared, bool drawNewHand) {
    score_ += calculateClearScore(linesCleared);
    handUsed_[handIndex] = true;
    
    bool allPlaced = std::all_of(handUsed_.begin(), handUsed_.end(), [](bool b) { return b; });
    
    if (allPlaced && drawNewHand) {
        drawHand();
    } else if (!allPlaced) {
        checkGameOver();
    }
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

} // namespace BlockGame
