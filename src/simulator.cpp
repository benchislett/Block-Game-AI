#include "game.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <chrono>
#include <iomanip>
#include <array>
#include <limits>

using namespace BlockGame;

/**
 * Simple strategy: pick a random legal move for any available piece
 */
class RandomStrategy {
public:
    explicit RandomStrategy(uint64_t seed) : rng_(seed) {}

    // Play a complete game, returns final score
    int playGame() {
        Game game(rng_());
        
        while (!game.isGameOver()) {
            auto moves = game.getAllLegalMoves();
            if (moves.empty()) break;
            
            // Pick a random legal move
            std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
            const Move& move = moves[dist(rng_)];
            game.makeMove(move);
        }
        
        return game.score();
    }

private:
    std::mt19937_64 rng_;
};

/**
 * Smart strategy with look-ahead and board evaluation
 * 
 * Key ideas:
 * - Evaluate board state based on multiple heuristics
 * - Look ahead through all 3 pieces in current hand
 * - Heavy penalty for states that lead to game over
 * - Reward clearing lines and keeping board "playable"
 */
class SmartStrategy {
public:
    explicit SmartStrategy(uint64_t seed) : rng_(seed) {}

    int playGame() {
        Game game(rng_());
        
        while (!game.isGameOver()) {
            Move bestMove = findBestMove(game);
            if (bestMove.pieceIndex < 0) break;
            game.makeMove(bestMove);
        }
        
        return game.score();
    }

private:
    std::mt19937_64 rng_;
    
    // Weights for evaluation function (tunable)
    static constexpr double W_POPULATION = -0.5;      // Penalty per filled cell
    static constexpr double W_HOLES = -5.0;           // Penalty for isolated empty cells
    static constexpr double W_CLEAR_BONUS = 20.0;     // Bonus per line cleared (squared)
    static constexpr double W_GAME_OVER = -10000.0;   // Massive penalty for losing
    static constexpr double W_EDGE_BONUS = 0.2;       // Bonus for filling edges
    static constexpr double W_NEAR_COMPLETE = 3.0;    // Bonus for rows/cols near completion
    
    // Fast popcount
    static inline int popcount(uint64_t x) {
        return __builtin_popcountll(x);
    }
    
    // Evaluate a board state - higher is better (optimized version)
    double evaluateBoard(Board::Mask boardMask) const {
        double score = 0.0;
        
        // 1. Population penalty - fewer filled cells is generally better
        int filled = popcount(boardMask);
        score += W_POPULATION * filled;
        
        // 2. Holes penalty using bit manipulation (much faster)
        // Shift board in all 4 directions, count empty cells surrounded by filled
        uint64_t empty = ~boardMask;
        uint64_t up = (boardMask >> 8) | 0xFF00000000000000ULL;    // Filled above (top row = edge)
        uint64_t down = (boardMask << 8) | 0x00000000000000FFULL;  // Filled below
        uint64_t left = ((boardMask >> 1) & 0x7F7F7F7F7F7F7F7FULL) | 0x0101010101010101ULL;
        uint64_t right = ((boardMask << 1) & 0xFEFEFEFEFEFEFEFEULL) | 0x8080808080808080ULL;
        
        // A "hole" has 3+ filled neighbors
        uint64_t n1 = up & down;
        uint64_t n2 = left & right;
        uint64_t trapped = empty & n1 & (left | right);  // Vertical trap
        trapped |= empty & n2 & (up | down);              // Horizontal trap
        int holes = popcount(trapped);
        score += W_HOLES * holes;
        
        // 3. Near-complete rows/columns bonus (7/8 filled = about to clear)
        for (int r = 0; r < 8; ++r) {
            int rowPop = popcount((boardMask >> (r * 8)) & 0xFF);
            if (rowPop >= 6) score += W_NEAR_COMPLETE * (rowPop - 5);
        }
        for (int c = 0; c < 8; ++c) {
            uint64_t colMask = 0x0101010101010101ULL << c;
            int colPop = popcount(boardMask & colMask);
            if (colPop >= 6) score += W_NEAR_COMPLETE * (colPop - 5);
        }
        
        // 4. Edge filling bonus - filling edges helps clear lines
        uint64_t edges = (boardMask & 0xFFULL) | ((boardMask >> 56) & 0xFF) |
                        (boardMask & 0x0101010101010101ULL) | (boardMask & 0x8080808080808080ULL);
        score += W_EDGE_BONUS * popcount(edges);
        
        return score;
    }
    
    // Simulate placing a piece on board, clear lines, return new board and lines cleared
    std::pair<Board::Mask, int> simulatePlace(Board::Mask boardMask, Board::Mask pieceMask) const {
        boardMask |= pieceMask;
        
        // Check and clear full rows/cols
        int linesCleared = 0;
        
        // Check rows
        for (int r = 0; r < 8; ++r) {
            uint64_t rowMask = 0xFFULL << (r * 8);
            if ((boardMask & rowMask) == rowMask) {
                boardMask &= ~rowMask;
                linesCleared++;
            }
        }
        
        // Check columns
        for (int c = 0; c < 8; ++c) {
            uint64_t colMask = 0x0101010101010101ULL << c;
            if ((boardMask & colMask) == colMask) {
                boardMask &= ~colMask;
                linesCleared++;
            }
        }
        
        return {boardMask, linesCleared};
    }
    
    // Check if a specific piece can be placed anywhere
    bool canPlacePiece(Board::Mask boardMask, int pieceIdx) const {
        const Piece& piece = getPiece(pieceIdx);
        const int maxRow = 8 - piece.height;
        const int maxCol = 8 - piece.width;
        for (int r = 0; r <= maxRow; ++r) {
            for (int c = 0; c <= maxCol; ++c) {
                Board::Mask mask = piece.shiftTo(r, c);
                if ((boardMask & mask) == 0) return true;
            }
        }
        return false;
    }
    
    // Check if we can complete the turn (place all remaining pieces)
    bool canCompleteTurn(Board::Mask boardMask, const std::array<int, 3>& hand,
                         const std::array<bool, 3>& used) const {
        // Quick check: can each remaining piece be placed independently?
        for (int i = 0; i < 3; ++i) {
            if (!used[i] && !canPlacePiece(boardMask, hand[i])) {
                return false;
            }
        }
        return true;
    }
    
    // Evaluate remaining moves greedily (fast approximation)
    double evaluateRemainingGreedy(Board::Mask boardMask, 
                                   const std::array<int, 3>& hand,
                                   std::array<bool, 3> used) const {
        double totalScore = 0.0;
        
        // Greedily place remaining pieces
        for (int pass = 0; pass < 3; ++pass) {
            int bestPiece = -1;
            int bestR = 0, bestC = 0;
            double bestEval = -std::numeric_limits<double>::infinity();
            Board::Mask bestMask = 0;
            int bestLines = 0;
            
            for (int i = 0; i < 3; ++i) {
                if (used[i]) continue;
                
                const Piece& piece = getPiece(hand[i]);
                const int maxRow = 8 - piece.height;
                const int maxCol = 8 - piece.width;
                
                for (int r = 0; r <= maxRow; ++r) {
                    for (int c = 0; c <= maxCol; ++c) {
                        Board::Mask mask = piece.shiftTo(r, c);
                        if ((boardMask & mask) != 0) continue;
                        
                        auto [newBoard, lines] = simulatePlace(boardMask, mask);
                        double eval = W_CLEAR_BONUS * lines * lines + evaluateBoard(newBoard);
                        
                        if (eval > bestEval) {
                            bestEval = eval;
                            bestPiece = i;
                            bestR = r;
                            bestC = c;
                            bestMask = mask;
                            bestLines = lines;
                        }
                    }
                }
            }
            
            if (bestPiece < 0) {
                // Can't place any more - check if it's game over
                for (int i = 0; i < 3; ++i) {
                    if (!used[i]) return W_GAME_OVER;
                }
                break;
            }
            
            // Apply best move
            auto [newBoard, _] = simulatePlace(boardMask, bestMask);
            boardMask = newBoard;
            totalScore += W_CLEAR_BONUS * bestLines * bestLines;
            used[bestPiece] = true;
        }
        
        return totalScore + evaluateBoard(boardMask);
    }
    
    // Find the best move for current game state
    Move findBestMove(const Game& game) {
        const auto& hand = game.hand();
        const auto& used = game.handUsed();
        Board::Mask boardMask = game.board().data();
        
        Move bestMove{-1, 0, 0, 0};
        double bestScore = -std::numeric_limits<double>::infinity();
        
        // Try each unused piece
        for (int i = 0; i < 3; ++i) {
            if (used[i]) continue;
            
            const Piece& piece = getPiece(hand[i]);
            const int maxRow = 8 - piece.height;
            const int maxCol = 8 - piece.width;
            
            for (int r = 0; r <= maxRow; ++r) {
                for (int c = 0; c <= maxCol; ++c) {
                    Board::Mask mask = piece.shiftTo(r, c);
                    if ((boardMask & mask) != 0) continue;
                    
                    // Simulate this move
                    auto [newBoard, linesCleared] = simulatePlace(boardMask, mask);
                    double moveScore = W_CLEAR_BONUS * linesCleared * linesCleared;
                    
                    // Look ahead with remaining pieces (greedy evaluation)
                    std::array<bool, 3> newUsed = used;
                    newUsed[i] = true;
                    
                    moveScore += evaluateRemainingGreedy(newBoard, hand, newUsed);
                    
                    if (moveScore > bestScore) {
                        bestScore = moveScore;
                        bestMove = {i, r, c, mask};
                    }
                }
            }
        }
        
        return bestMove;
    }
};

/**
 * Statistics calculator
 */
struct Statistics {
    double min;      // P0
    double p10;
    double p25;
    double median;   // P50
    double p75;
    double p90;
    double max;      // P100
    double mean;
    double stddev;

    static Statistics compute(std::vector<int>& scores) {
        Statistics stats{};
        
        if (scores.empty()) return stats;
        
        // Sort for percentiles
        std::sort(scores.begin(), scores.end());
        
        size_t n = scores.size();
        
        // Percentiles
        stats.min = scores[0];
        stats.p10 = percentile(scores, 10);
        stats.p25 = percentile(scores, 25);
        stats.median = percentile(scores, 50);
        stats.p75 = percentile(scores, 75);
        stats.p90 = percentile(scores, 90);
        stats.max = scores[n - 1];
        
        // Mean
        double sum = std::accumulate(scores.begin(), scores.end(), 0.0);
        stats.mean = sum / n;
        
        // Standard deviation
        double sqSum = 0;
        for (int s : scores) {
            double diff = s - stats.mean;
            sqSum += diff * diff;
        }
        stats.stddev = std::sqrt(sqSum / n);
        
        return stats;
    }

private:
    static double percentile(const std::vector<int>& sorted, double p) {
        if (sorted.empty()) return 0;
        if (sorted.size() == 1) return sorted[0];
        
        double idx = (p / 100.0) * (sorted.size() - 1);
        size_t lo = static_cast<size_t>(idx);
        size_t hi = lo + 1;
        double frac = idx - lo;
        
        if (hi >= sorted.size()) return sorted[lo];
        return sorted[lo] * (1 - frac) + sorted[hi] * frac;
    }
};

void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " [strategy] [num_runs]\n";
    std::cerr << "  strategy: 'random' or 'smart' (default: smart)\n";
    std::cerr << "  num_runs: Number of simulation runs (default: 1000)\n";
}

int main(int argc, char* argv[]) {
    int numRuns = 1000;
    std::string strategyName = "smart";
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "random" || arg == "smart") {
            strategyName = arg;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            try {
                numRuns = std::stoi(arg);
                if (numRuns <= 0) {
                    std::cerr << "Error: num_runs must be positive\n";
                    return 1;
                }
            } catch (const std::exception& e) {
                printUsage(argv[0]);
                return 1;
            }
        }
    }
    
    std::cout << "Running " << numRuns << " simulations with " << strategyName << " strategy...\n";
    std::cout << std::flush;
    
    // Seed from random device
    std::random_device rd;
    uint64_t seed = rd();
    
    // Run simulations
    std::vector<int> scores;
    scores.reserve(numRuns);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (strategyName == "random") {
        RandomStrategy strategy(seed);
        for (int i = 0; i < numRuns; ++i) {
            int score = strategy.playGame();
            scores.push_back(score);
            
            // Progress indicator
            if (numRuns >= 10 && ((i + 1) % (numRuns / 10) == 0 || i == numRuns - 1)) {
                int pct = (i + 1) * 100 / numRuns;
                std::cout << "\r  Progress: " << std::setw(3) << pct << "% (" << (i + 1) << "/" << numRuns << ")" << std::flush;
            }
        }
    } else {
        SmartStrategy strategy(seed);
        for (int i = 0; i < numRuns; ++i) {
            int score = strategy.playGame();
            scores.push_back(score);
            
            // Progress indicator
            if (numRuns >= 10 && ((i + 1) % (numRuns / 10) == 0 || i == numRuns - 1)) {
                int pct = (i + 1) * 100 / numRuns;
                std::cout << "\r  Progress: " << std::setw(3) << pct << "% (" << (i + 1) << "/" << numRuns << ")" << std::flush;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\n\n";
    
    // Compute and display statistics
    Statistics stats = Statistics::compute(scores);
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "═══════════════════════════════════════════\n";
    std::cout << "           SIMULATION RESULTS              \n";
    std::cout << "═══════════════════════════════════════════\n";
    std::cout << "  Strategy:    " << strategyName << "\n";
    std::cout << "  Runs:        " << numRuns << "\n";
    std::cout << "  Time:        " << duration.count() << " ms\n";
    std::cout << "  Games/sec:   " << (numRuns * 1000.0 / duration.count()) << "\n";
    std::cout << "───────────────────────────────────────────\n";
    std::cout << "  P0   (min):  " << std::setw(10) << stats.min << "\n";
    std::cout << "  P10:         " << std::setw(10) << stats.p10 << "\n";
    std::cout << "  P25:         " << std::setw(10) << stats.p25 << "\n";
    std::cout << "  P50  (med):  " << std::setw(10) << stats.median << "\n";
    std::cout << "  P75:         " << std::setw(10) << stats.p75 << "\n";
    std::cout << "  P90:         " << std::setw(10) << stats.p90 << "\n";
    std::cout << "  P100 (max):  " << std::setw(10) << stats.max << "\n";
    std::cout << "───────────────────────────────────────────\n";
    std::cout << "  Mean:        " << std::setw(10) << stats.mean << "\n";
    std::cout << "  Std Dev:     " << std::setw(10) << stats.stddev << "\n";
    std::cout << "═══════════════════════════════════════════\n";
    
    return 0;
}
