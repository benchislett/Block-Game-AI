#pragma once

#include "game.hpp"
#include <string>

namespace BlockGame {

/**
 * Terminal-based user interface for playing the game
 */
class TerminalUI {
public:
    TerminalUI();

    // Run the game loop
    void run();

private:
    Game game_;
    int selectedHandIndex_;
    int cursorRow_;
    int cursorCol_;

    // Display functions
    void clearScreen();
    void displayGame();
    void displayBoard();
    void displayHand();
    void displayPiecePreview(int handIndex);
    void displayStatus();
    void displayHelp();
    void displayGameOver();

    // Input handling
    std::string getInput();
    bool processCommand(const std::string& input);
    bool parseMove(const std::string& input, int& handIndex, int& row, int& col);

    // Colors and formatting
    static const char* RESET;
    static const char* BOLD;
    static const char* DIM;
    static const char* RED;
    static const char* GREEN;
    static const char* YELLOW;
    static const char* BLUE;
    static const char* MAGENTA;
    static const char* CYAN;
    static const char* BG_GRAY;
};

} // namespace BlockGame
