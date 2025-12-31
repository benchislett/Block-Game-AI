# README

Vibe-coded playground, mostly evaluating Claude Opus 4.5 Agent mode skills.

## Instructions for Agent

I want you to create a CMake/C++ project implementing a simple game.

The game has an 8x8 board of "squares" that are either full or empty. It starts as fully empty.
The board should be represented in memory using a single 64-bit value, for efficiency. As
you will see, the only operations required to operate on this board are bit-mask AND and OR operations and simple compares.

The game proceeds in "turns". Each turn, the player is assigned a hand of three "pieces", selected
randomly with duplicates permitted, from the pool of possible pieces. Then the player must place
each of the pieces on the board, in any order they choose, until they either successfully place all three
pieces or can no longer place a piece. A piece may only be placed if it lies fully within the board and if
each of the destination squares on the board are unoccupied. If no placements are possible, the game ends.

After placing a piece, all fully populated columns and rows are cleared. Then the score is increased
by `(num_rows_and_cols_cleared ** 2) * 8`. One cleared row grants 8 points, one row and one column grants `4*8 == 32` points, three rows cleared grants `9*8 == 72` points.

The goal of the game is to get as high a score as possible before the game ends (when you are unable to place any of your remaining assigned pieces).

The game pieces are distinct and cannot be rotated during play. For non-symmetric pieces, each rotation is treated as a distinct piece. Do not implement any rotation logic, instead
just implement the pieces as distinct entries in a list of possible piece choices.

The pieces in the game are:
- 2x2 Square
- 3x3 Square
- 2x3 Rectangle
- 3x2 Rectangle
- 3x1 Line
- 4x1 Line
- 5x1 Line
- 1x3 Line
- 1x4 Line
- 1x5 Line
- 'S' piece from Tetris, all 4 rotations
- 'T' piece from Tetris, all 4 rotations
- 2x2 square minus one corner (aka small corner piece), all 4 rotations
- 3x3 square minus 2x2 square from one corner (aka large corner piece), all 4 rotations
- Tetris 'L' piece and mirror, all 8 combinations (2 mirror x 4 rotations)

Implement each piece as a bitmask and implement highly efficient legal move generation, as this will be used for deep simulations for AI gameplay.

Next, implement a simple frontend that I can use to 'play' the game, for the purpose of ensuring correctness and debugging move selection.
It should have a pretty and usable terminal interface (no GUI elements or libraries). Ensure that the core game logic is reused, even if at the detriment of the UI/UX, such that minimal code is duplicated and the core engine maintains peak performance characteristics.

Add support in the CLI for an "auto" mode that will automatically move a piece to a random legal move when I say "auto <piece>". It should print the move it has selected.

Next, make a new executable target called "simulator". It will implement a simple strategy function, for now it just chooses any legal move for any available tile until the game ends. Then it reports the score. It can run for a configurable (CLI argument) number of runs, and will report the P0 (min) P10, P25, P50 (mean), P75, P90, P100 (max), median, and stddev score over the runs