#include "game.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <chrono>
#include <iomanip>

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
    std::cerr << "Usage: " << prog << " [num_runs]\n";
    std::cerr << "  num_runs: Number of simulation runs (default: 1000)\n";
}

int main(int argc, char* argv[]) {
    int numRuns = 1000;
    
    if (argc > 1) {
        try {
            numRuns = std::stoi(argv[1]);
            if (numRuns <= 0) {
                std::cerr << "Error: num_runs must be positive\n";
                return 1;
            }
        } catch (const std::exception& e) {
            printUsage(argv[0]);
            return 1;
        }
    }
    
    std::cout << "Running " << numRuns << " simulations with random strategy...\n";
    std::cout << std::flush;
    
    // Seed from random device
    std::random_device rd;
    RandomStrategy strategy(rd());
    
    // Run simulations
    std::vector<int> scores;
    scores.reserve(numRuns);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numRuns; ++i) {
        int score = strategy.playGame();
        scores.push_back(score);
        
        // Progress indicator every 10%
        if ((i + 1) % (numRuns / 10) == 0 || i == numRuns - 1) {
            int pct = (i + 1) * 100 / numRuns;
            std::cout << "\r  Progress: " << std::setw(3) << pct << "% (" << (i + 1) << "/" << numRuns << ")" << std::flush;
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
