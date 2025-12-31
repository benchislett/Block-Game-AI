#include "terminal_ui.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <random>

namespace BlockGame {

// ANSI escape codes
const char* TerminalUI::RESET = "\033[0m";
const char* TerminalUI::BOLD = "\033[1m";
const char* TerminalUI::DIM = "\033[2m";
const char* TerminalUI::RED = "\033[31m";
const char* TerminalUI::GREEN = "\033[32m";
const char* TerminalUI::YELLOW = "\033[33m";
const char* TerminalUI::BLUE = "\033[34m";
const char* TerminalUI::MAGENTA = "\033[35m";
const char* TerminalUI::CYAN = "\033[36m";
const char* TerminalUI::BG_GRAY = "\033[48;5;240m";

TerminalUI::TerminalUI() 
    : game_()
    , selectedHandIndex_(0)
    , cursorRow_(0)
    , cursorCol_(0) {
}

void TerminalUI::clearScreen() {
    std::cout << "\033[2J\033[H";
}

void TerminalUI::run() {
    clearScreen();
    
    while (!game_.isGameOver()) {
        displayGame();
        
        std::string input = getInput();
        if (input == "quit" || input == "q" || input == "exit") {
            std::cout << "\nThanks for playing! Final score: " << game_.score() << "\n";
            return;
        }
        
        processCommand(input);
    }
    
    displayGame();
    displayGameOver();
}

void TerminalUI::displayGame() {
    clearScreen();
    
    std::cout << BOLD << CYAN;
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              " << YELLOW << "⬛ BLOCK PUZZLE GAME ⬛" << CYAN << "                          ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
    
    displayStatus();
    std::cout << "\n";
    displayBoard();
    std::cout << "\n";
    displayHand();
    std::cout << "\n";
    displayHelp();
}

void TerminalUI::displayStatus() {
    std::cout << BOLD << "  Turn: " << YELLOW << game_.turnNumber() << RESET;
    std::cout << "    ";
    std::cout << BOLD << "Score: " << GREEN << game_.score() << RESET;
    std::cout << "\n";
}

void TerminalUI::displayBoard() {
    const Board& board = game_.board();
    
    std::cout << BOLD << "  Board:" << RESET << "\n";
    std::cout << "     ";
    for (int col = 0; col < 8; ++col) {
        std::cout << DIM << col << " " << RESET;
    }
    std::cout << "\n";
    
    std::cout << "    ╔";
    for (int i = 0; i < 16; ++i) std::cout << "═";
    std::cout << "╗\n";
    
    for (int row = 0; row < 8; ++row) {
        std::cout << "  " << DIM << row << RESET << " ║";
        for (int col = 0; col < 8; ++col) {
            if (board.isOccupied(row, col)) {
                std::cout << BLUE << "██" << RESET;
            } else {
                // Checkerboard pattern for empty squares
                if ((row + col) % 2 == 0) {
                    std::cout << "· ";
                } else {
                    std::cout << "  ";
                }
            }
        }
        std::cout << "║\n";
    }
    
    std::cout << "    ╚";
    for (int i = 0; i < 16; ++i) std::cout << "═";
    std::cout << "╝\n";
}

void TerminalUI::displayHand() {
    const auto& hand = game_.hand();
    const auto& used = game_.handUsed();
    
    std::cout << BOLD << "  Your pieces:" << RESET << "\n\n";
    
    // Display piece names
    std::cout << "  ";
    for (int i = 0; i < Game::HAND_SIZE; ++i) {
        std::string label = "[" + std::to_string(i + 1) + "] ";
        if (used[i]) {
            std::cout << DIM << label << "USED" << RESET;
        } else {
            const Piece& piece = getPiece(hand[i]);
            std::cout << BOLD << label << RESET << piece.name;
        }
        std::cout << "          ";
    }
    std::cout << "\n\n";
    
    // Find max height for display
    int maxHeight = 0;
    for (int i = 0; i < Game::HAND_SIZE; ++i) {
        if (!used[i]) {
            maxHeight = std::max(maxHeight, getPiece(hand[i]).height);
        }
    }
    
    // Display pieces row by row
    for (int row = 0; row < maxHeight; ++row) {
        std::cout << "      ";
        for (int i = 0; i < Game::HAND_SIZE; ++i) {
            if (used[i]) {
                std::cout << "                      ";
            } else {
                const Piece& piece = getPiece(hand[i]);
                if (row < piece.height) {
                    for (int col = 0; col < 5; ++col) {  // Max width display
                        if (col < piece.width && (piece.baseMask & (1ULL << (row * 8 + col)))) {
                            std::cout << MAGENTA << "██" << RESET;
                        } else {
                            std::cout << "  ";
                        }
                    }
                    std::cout << "            ";
                } else {
                    std::cout << "                      ";
                }
            }
        }
        std::cout << "\n";
    }
}

void TerminalUI::displayHelp() {
    std::cout << DIM << "  ─────────────────────────────────────────────────────────────\n" << RESET;
    std::cout << BOLD << "  Commands:" << RESET << "\n";
    std::cout << "    " << CYAN << "<piece> <row> <col>" << RESET << " - Place piece (e.g., '1 3 4' places piece 1 at row 3, col 4)\n";
    std::cout << "    " << CYAN << "auto <piece>" << RESET << "         - Auto-place piece at random legal position\n";
    std::cout << "    " << CYAN << "moves <piece>" << RESET << "        - Show all valid moves for a piece\n";
    std::cout << "    " << CYAN << "help" << RESET << "                 - Show all commands\n";
    std::cout << "    " << CYAN << "quit" << RESET << "                 - Exit the game\n";
    std::cout << "\n";
}

void TerminalUI::displayGameOver() {
    std::cout << "\n";
    std::cout << RED << BOLD;
    std::cout << "  ╔═══════════════════════════════════════════╗\n";
    std::cout << "  ║              GAME OVER!                   ║\n";
    std::cout << "  ╚═══════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
    std::cout << "  Final Score: " << BOLD << GREEN << game_.score() << RESET << "\n";
    std::cout << "  Turns Played: " << game_.turnNumber() << "\n";
    std::cout << "\n  No more valid moves available.\n\n";
}

std::string TerminalUI::getInput() {
    std::cout << BOLD << "  > " << RESET;
    std::string input;
    std::getline(std::cin, input);
    
    // Trim whitespace
    size_t start = input.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t");
    return input.substr(start, end - start + 1);
}

bool TerminalUI::processCommand(const std::string& input) {
    if (input.empty()) {
        return true;
    }
    
    // Convert to lowercase for command matching
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    // Help command
    if (lower == "help" || lower == "h" || lower == "?") {
        std::cout << "\n" << BOLD << "  All Commands:" << RESET << "\n";
        std::cout << "    " << CYAN << "<piece> <row> <col>" << RESET << " - Place piece at position\n";
        std::cout << "       Example: '1 3 4' places piece 1 at row 3, column 4\n";
        std::cout << "       Pieces are numbered 1-3, rows/cols are 0-7\n";
        std::cout << "    " << CYAN << "auto <piece>" << RESET << "         - Auto-place piece at random legal position\n";
        std::cout << "       Example: 'auto 1' places piece 1 at a random valid spot\n";
        std::cout << "    " << CYAN << "moves <piece>" << RESET << "        - Show all valid positions for a piece\n";
        std::cout << "    " << CYAN << "moves" << RESET << "                - Show count of valid moves for all pieces\n";
        std::cout << "    " << CYAN << "new" << RESET << "                  - Start a new game\n";
        std::cout << "    " << CYAN << "quit / q" << RESET << "             - Exit the game\n";
        std::cout << "\n  Press Enter to continue...";
        std::cin.get();
        return true;
    }
    
    // New game
    if (lower == "new" || lower == "restart") {
        game_.reset();
        return true;
    }
    
    // Show moves
    if (lower.substr(0, 5) == "moves") {
        if (lower.length() > 5) {
            // Specific piece
            int pieceNum = std::stoi(lower.substr(6)) - 1;
            if (pieceNum >= 0 && pieceNum < Game::HAND_SIZE && !game_.handUsed()[pieceNum]) {
                auto moves = game_.getLegalMoves(pieceNum);
                std::cout << "\n  " << BOLD << "Valid positions for piece " << (pieceNum + 1) << ":" << RESET << "\n  ";
                for (const auto& move : moves) {
                    std::cout << "(" << move.row << "," << move.col << ") ";
                }
                std::cout << "\n  Total: " << moves.size() << " positions\n";
            } else {
                std::cout << RED << "  Invalid piece number or piece already used.\n" << RESET;
            }
        } else {
            // All pieces
            std::cout << "\n  " << BOLD << "Valid moves per piece:" << RESET << "\n";
            for (int i = 0; i < Game::HAND_SIZE; ++i) {
                if (!game_.handUsed()[i]) {
                    auto moves = game_.getLegalMoves(i);
                    std::cout << "    Piece " << (i + 1) << ": " << moves.size() << " positions\n";
                }
            }
        }
        std::cout << "\n  Press Enter to continue...";
        std::cin.get();
        return true;
    }
    
    // Auto move - place piece at random legal position
    if (lower.substr(0, 4) == "auto") {
        if (lower.length() > 4) {
            int pieceNum = std::stoi(lower.substr(5)) - 1;
            if (pieceNum >= 0 && pieceNum < Game::HAND_SIZE && !game_.handUsed()[pieceNum]) {
                auto moves = game_.getLegalMoves(pieceNum);
                if (moves.empty()) {
                    std::cout << RED << "  No valid moves for piece " << (pieceNum + 1) << "!\n" << RESET;
                } else {
                    // Select random move
                    static std::mt19937 rng(std::random_device{}());
                    std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
                    const Move& selected = moves[dist(rng)];
                    
                    std::cout << YELLOW << "  Auto-placing piece " << (pieceNum + 1) 
                              << " at (" << selected.row << ", " << selected.col << ")" << RESET << "\n";
                    
                    int points = game_.placePiece(pieceNum, selected.row, selected.col);
                    if (points > 0) {
                        std::cout << GREEN << "  +" << points << " points from clearing lines!" << RESET << "\n";
                    }
                }
            } else {
                std::cout << RED << "  Invalid piece number or piece already used.\n" << RESET;
            }
        } else {
            std::cout << RED << "  Usage: auto <piece>  (e.g., 'auto 1')\n" << RESET;
        }
        std::cout << "  Press Enter to continue...";
        std::cin.get();
        return true;
    }
    
    // Try to parse as move: <piece> <row> <col>
    int handIndex, row, col;
    if (parseMove(input, handIndex, row, col)) {
        if (game_.canPlace(handIndex, row, col)) {
            int points = game_.placePiece(handIndex, row, col);
            if (points > 0) {
                std::cout << GREEN << "  +" << points << " points from clearing lines!" << RESET << "\n";
                std::cout << "  Press Enter to continue...";
                std::cin.get();
            }
        } else {
            std::cout << RED << "  Cannot place piece there! (occupied or out of bounds)\n" << RESET;
            std::cout << "  Press Enter to continue...";
            std::cin.get();
        }
        return true;
    }
    
    std::cout << RED << "  Unknown command. Type 'help' for available commands.\n" << RESET;
    std::cout << "  Press Enter to continue...";
    std::cin.get();
    return true;
}

bool TerminalUI::parseMove(const std::string& input, int& handIndex, int& row, int& col) {
    std::istringstream iss(input);
    int piece;
    
    if (!(iss >> piece >> row >> col)) {
        return false;
    }
    
    // Convert 1-indexed piece to 0-indexed
    handIndex = piece - 1;
    
    // Validate ranges
    if (handIndex < 0 || handIndex >= Game::HAND_SIZE) {
        std::cout << RED << "  Piece must be 1-" << Game::HAND_SIZE << "\n" << RESET;
        return false;
    }
    
    if (game_.handUsed()[handIndex]) {
        std::cout << RED << "  Piece " << piece << " has already been used this turn.\n" << RESET;
        return false;
    }
    
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        std::cout << RED << "  Row and column must be 0-7\n" << RESET;
        return false;
    }
    
    return true;
}

} // namespace BlockGame
