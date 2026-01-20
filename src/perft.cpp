#include "board.hpp"
#include "pieces.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <map>
#include <string>
#include <unordered_map>
#include <cassert>

using namespace BlockGame;

// Mode definitions
enum class Mode {
    DEFAULT,
    NEARFULL
};

struct TranspositionTable {
    // Map from board hash to terminal node count
    // One for each depth, up to a fixed max depth
    constexpr static int MAX_DEPTH = 8;
    std::array<std::unordered_map<uint64_t, uint64_t>, MAX_DEPTH> tables;

    [[nodiscard]] int64_t query(uint64_t board_hash, int depth) const {
        assert (depth >= 0 && depth < static_cast<int>(tables.size()));
        auto it = tables[depth].find(board_hash);
        if (it != tables[depth].end()) {
            return it->second;
        }
        return -1;
    }

    void store(uint64_t board_hash, int depth, uint64_t count) {
        assert (depth >= 0 && depth < static_cast<int>(tables.size()));
        tables[depth][board_hash] = count;
    }

    void clear() {
        for (auto& table : tables) {
            table.clear();
        }
    }
};

// Recursive function to count terminal nodes
// A node is terminal if:
// 1. depth == max_depth
// 2. OR no legal moves possible from current state (game over)
uint64_t countTerminalNodes(const Board& board, int depth, int max_depth, const std::vector<Piece>& pieces, TranspositionTable& tt) {
    // If max depth reached, this path ends here.
    if (depth == max_depth) {
        return 1;
    }

    if (depth < TranspositionTable::MAX_DEPTH) {
        const int64_t tt_result = tt.query(board.data(), depth);
        if (tt_result != -1) {
            return tt_result;
        }
    }

    uint64_t total_terminal_nodes = 0;

    // Try all pieces
    for (const auto& piece : pieces) {
        // Try all positions
        for (int row = 0; row < piece.shiftTable.maxRow + 1; ++row) {
            for (int col = 0; col < piece.shiftTable.maxCol + 1; ++col) {
                const uint64_t mask = piece.shiftToUnsafe(row, col);

                // Check if placement fits on the board
                if (board.canPlace(mask)) {
                    
                    Board next_board = board;
                    next_board.placeAndClear(mask);
                    
                    total_terminal_nodes += countTerminalNodes(next_board, depth + 1, max_depth, pieces, tt);
                }
            }
        }
    }

    // If no moves were possible, this is a terminal node (game over state)
    if (total_terminal_nodes == 0) {
        total_terminal_nodes = 1;
    }
    
    if (depth < TranspositionTable::MAX_DEPTH) {
        tt.store(board.data(), depth, total_terminal_nodes);
    }

    return total_terminal_nodes;
}

int main(int argc, char* argv[]) {
    int max_depth_limit = 2; // Default
    Mode mode = Mode::DEFAULT;
    std::string modeStr = "default";

    if (argc > 1) {
        try {
            max_depth_limit = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid argument for max depth\n";
            return 1;
        }
    }
    
    if (argc > 2) {
        modeStr = argv[2];
        if (modeStr == "nearfull") {
            mode = Mode::NEARFULL;
        } else if (modeStr != "default") {
            std::cerr << "Unknown mode: " << modeStr << ". Using default.\n";
            modeStr = "default";
        }
    }
    
    std::cout << "Running Perft (Terminal Node Count)\n";
    std::cout << "  Max Depth: " << max_depth_limit << "\n";
    std::cout << "  Mode:      " << modeStr << "\n\n";
    
    // Initialize board based on mode
    Board initialBoard;
    if (mode == Mode::NEARFULL) {
        // Fill all but top/bottom row and left/right col
        // Rows 1-6, Cols 1-6
        for (int r = 1; r <= 6; ++r) {
            for (int c = 1; c <= 6; ++c) {
                initialBoard.setOccupied(r, c);
            }
        }
    }

    // Pre-fetch all pieces
    const auto& pieces = getAllPieces();

    // Baseline results (Hardcoded for regression testing)
    // Key: Mode string + ":" + Depth -> Count
    const std::map<std::string, uint64_t> BASELINE = {
        {"default:0", 1},
        {"default:1", 1421},
        {"default:2", 1617196},
        {"default:3", 1455574952},
        {"nearfull:0", 1},
        {"nearfull:1", 76},
        {"nearfull:2", 4380},
        {"nearfull:3", 507036},
        {"nearfull:4", 142586120},
    };

    TranspositionTable tt;

    auto total_start = std::chrono::high_resolution_clock::now();

    // We run for the specific max_depth requested.
    // The user prompt implied: "for each depth from 0 up to max_depth".
    // This usually means running the full perft for d=0, then d=1, ... d=max_depth.
    
    bool all_passed = true;

    for (int d = 0; d <= max_depth_limit; ++d) {
        auto depth_start = std::chrono::high_resolution_clock::now();
        
        uint64_t count = countTerminalNodes(initialBoard, 0, d, pieces, tt);
        
        auto depth_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = depth_end - depth_start;
        
        std::string key = modeStr + ":" + std::to_string(d);
        
        std::cout << "Depth " << d << ": " << std::setw(12) << count 
                  << " nodes (" << std::fixed << std::setprecision(3) << diff.count() << "s)";

        if (BASELINE.count(key)) {
            uint64_t expected = BASELINE.at(key);
            if (count == expected) {
                std::cout << " [PASS]";
            } else {
                std::cout << " [FAIL] Expected " << expected;
                all_passed = false;
            }
        } else {
            std::cout << " [NEW]";
        }
        std::cout << "\n";
        tt.clear();
    }
    
    return all_passed ? 0 : 1;
}
