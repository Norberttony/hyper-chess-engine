
# Hyper Active
Hyper Active is a chess variant engine specifically designed and optimized for Hyper Chess, a variant of Ultima Chess. This repository is a CLI that allows users to analyze Hyper Chess positions. It is possible to play against the latest release of the engine with a graphical user interface [here](https://norberttony.github.io/local-hyper-chess/#lobby).

The [linked website](https://www.carusos.org/Hyperchess/hyperchess.html) provides the rules of Hyper Chess. The non-MIT rules (where the death squares formed by the king and coordinator do not persist) are used.

Note: the liberty to not implement the fifty move rule or draw by material was taken.

## Build
This project requires a GNU compiler that supports the C11 standard and can be run using the `gcc` command, and the ability to run makefiles. Running `make` in the project directory will build the executable `bin/hyper-active.exe`. The makefile has been configured to automatically run the executable after building it. This has been tested to work on Windows and Linux.

Running `make web` in the project directory will build a web assembly version of the engine that exposes some of the engine's functionality to the web browser, generating `bin/hyper-active.js` and `bin/hyper-active.wasm`. This build requires a separate interface to properly use.

## UCI Commands
Here is a list of some of the UCI commands supported. These commands are useful for analyzing games.
- `d`: displays the current board using ASCII
- `position startpos`: sets up the starting position
- `position fen {fen}`: sets up the board to the given "FEN" (replace {fen} with your "FEN" string)
- `position moves {lan}`: plays the given long algebraic notation (LAN) moves on the board. For example, `position moves c2c5` moves the piece on `c2` to `c5`. Multiple moves must be separated by spaces like this: `position moves c2c5 g7g6`.
- `go`: the engine will begin to analyze the position, outputting depth, evaluation, and principal variation to the terminal. It is possible to stop the engine by typing `stop` or `quit`. The evaluation is always from white's perspective (ie. positive means white is better and negative means black is better).
- `go depth {depth}`: the engine will stop after reaching a depth of {depth} ply
- `go movetime {movetime}`: the enigne will stop after thinking for {movetime} milliseconds
- `quit`: closes the interface

Here is a list of some of the available advanced commands:
- `uci`: engine should respond with its identification and `uciok`
- `isready`: engine should respond with `readyok`
- `clear hash`: clears the engine's transposition tables
- `readfile {path}`: starts reading commands from the given file, with a caveat: any line containing only a line break will make the engine stop reading the file.

## Design
The engine can be broken up into four main components: board representation, move generation, search, and evaluation.

### Board Representation
The position is represented using an array of 16 bitboards. A bitboard is a 64-bit unsigned integer, and a chess board has 64 squares. This means that a 0 or a 1 can be stored for each square using a single 64-bit unsigned integer. 14 of the 16 bitboards are used to represent the existence of each piece type and color (7 piece types * 2 colors = 14). The remaining 2 bitboards are for having white's total occupancy and black's total occupancy (without having to OR each individual bitboard).

Bitboards make it difficult to see what type of piece is on a particular square, which is useful in move generation. For this reason, the engine also maintains an array indexed by square that returns the piece on that square.

### Move Generation
Bitboards are used for move generation, where a 1 indicates a piece can move to that square and 0 indicates the opposite. This engine uses a technique known as magic bitboards. Given that there is a rook on a square, any pieces on the same horizontal or vertical are potential blockers. A 64-bit unsigned integer n is found such that `(n * blockers) >> max_number_of_blockers_bits` is a unique number for any blockers configuration. This unique index can be used in look-up tables. This means that pseudo-legal moves are calculated in O(1) time.

Each move is encoded into a single 32 bit unsigned integer. Keeping moves as small as possible optimizes the cache (memory) for a small penalty in speed when decoding. Since memory tends to be slower than CPU operations, this encoding/decoding approach was chosen.

### Search
The engine searches the position tree using a depth-first search and the minmax algorithm. The minmax algorithm is further improved using alpha-beta pruning, which produces the same results but does not require every node to be searched (ie. some variations are pruned). Alpha-beta pruning benefits from effective move ordering, because each move sets a lower bound (alpha) and an upper bound (beta) on the score for the next moves. The engine uses killer moves and the history heuristic in its move ordering.

The search also makes use of transposition tables with 64-bit unsigned integer Zobrist hashing. Each entry in the transposition table stores the engine's evaluation of the position, allowing the engine to recall an earlier evaluation of the same position without having to re-search it. A zobrist hash of a position is a semi-unique identifier of the position that is calculated by XORing together distinct features of the position (including piece placement). This zobrist hash is used to access the transposition table.

Zobrist hashes stored in a circular buffer are also used for identifying when threefold repetitions occur.

### Evaluation
Using bitwise manipulation on bitboards provides a quick and easy way of extracting features out of the position. For instance, the engine identifies immobilized pieces using `kingMoves\[immSq\] & position[opposingSide]`. The engine also makes use of incremental evaluation techniques, such as maintaining a material and piece square table value instead of recalculating it every time.
