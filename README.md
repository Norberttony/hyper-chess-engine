
# Hyper Active
![hyper-active-terminal](https://github.com/user-attachments/assets/fe4a394e-6109-449f-a825-deeaaab9f61b)

Hyper Active is a chess variant engine specifically designed and optimized for Hyper Chess, a variant of Ultima Chess. This repository is a CLI that allows users to analyze Hyper Chess positions. It is possible to play against the latest release of the engine with a graphical user interface [here](https://norberttony.github.io/local-hyper-chess/#lobby).

The [linked website](https://www.carusos.org/Hyperchess/hyperchess.html) provides the rules of Hyper Chess. The non-MIT rules (where the death squares formed by the king and coordinator do not persist) are used.

Note: the liberty to not implement draw by material was taken.

## Build
This project requires a GNU compiler that supports the C11 standard and can be run using the `gcc` command, and the ability to run makefiles. Running `make` in the project directory will build the executable `bin/hyper-active.exe`. The makefile has been configured to automatically run the executable after building it. This has been tested to work on Windows and Linux.

Running `make web` in the project directory will build a web assembly version of the engine that exposes some of the engine's functionality to the web browser, generating `bin/hyper-active.js` and `bin/hyper-active.wasm`. This build requires a separate interface to properly use.

## UCI Commands
Here is a list of some of the UCI commands supported. These commands are useful for analyzing games.
- `d`: displays the current board using ASCII
- `position startpos`: sets up the starting position
- `position fen {fen}`: sets up the board to the given "FEN" (replace {fen} with your "FEN" string)
- `position moves {lan}`: plays the given long algebraic notation (LAN) moves on the board. For example, `position moves c2c5` moves the piece on `c2` to `c5`. Multiple moves must be separated by spaces like this: `position moves c2c5 g7g6`.
- It is also possible to both set the position and the moves in one single command: `position startpos moves g2g5 c7c4` will set to the starting position and then play the moves g2g5 and c7c4.
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
The engine can be broken up into four main components: board representation, move generation, search, and evaluation. Here is a quick overview of the engine's features:
- Bitboard board representation
- Alpha-beta pruning algorithm
- Transposition table
- TT move ordering, killer moves, and history heuristic
- Quiescent search only for capturing moves
- Piece square tables

### Board Representation
The position is represented using an array of 16 bitboards. A bitboard is a 64-bit unsigned integer, and a chess board has 64 squares. This means that a 0 or a 1 can be stored for each square using a single 64-bit unsigned integer. 14 of the 16 bitboards are used to represent the existence of each piece type and color (7 piece types * 2 colors = 14). The remaining 2 bitboards are for having white's total occupancy and black's total occupancy (without having to OR each individual bitboard).

Bitboards make it difficult to see what type of piece is on a particular square, which is useful in move generation. For this reason, the engine also maintains an array indexed by square that returns the piece on that square.

### Move Generation
Bitboards are used for move generation, where a 1 indicates a piece can move to that square and 0 indicates the opposite. This engine uses a technique known as magic bitboards. Given that there is a rook on a square, any pieces on the same horizontal or vertical are potential blockers. A 64-bit unsigned integer n is found such that `(n * blockers) >> max_number_of_blockers_bits` is a unique number for any blockers configuration. This unique index can be used in look-up tables. The bitboards that contain the pseudo-legal moves are calculated in O(1) time. Instead of scanning the bitboard to identify which squares are 1 bits on the bitboard, the code isolates the last bit, multiplies by a DeBruijn constant, and uses the result in a look-up table to get the square.

Each move is encoded into a single 32 bit unsigned integer. Keeping moves as small as possible optimizes the cache (memory) for a small penalty in speed when decoding. Since memory tends to be slower than CPU operations, this encoding and decoding approach was chosen.

In conclusion, the moves are ordered as follows:
1. The move recovered from the TT if it exists
2. Capture moves (ordered based on the amount of captured material)
3. The two killer moves
4. The rest of the quiet moves are ordered by their history heuristic

### Search
The engine searches the position tree using a depth-first search and the minmax algorithm. The minmax algorithm is further improved using alpha-beta pruning, which produces the same results but does not require every node to be searched (ie. some variations are pruned). Alpha-beta pruning benefits from effective move ordering, because each move sets a lower bound (alpha) and an upper bound (beta) on the score for the next moves. If a move exceeds these bounds, it guarantees that this line of play will not occur because one of the players has already a better line of play calculated earlier.

The engine favors captures (winning or losing) and uses killer moves and the history heuristic for quiet (non-capturing) moves. Killer moves are moves that caused a beta cut-off, and are tried again in sibling nodes of the tree. As an example, if the engine analyzes a position where the opponent has a dangerous threat (such as mate-in-one), then the threat is stored as a killer move (and is now scored more highly than other moves). When the engine tries playing another move in this position, the killer move is now tried much earlier. The engine stores at most two different killer moves. The history heuristic adds a score to quiet moves that caused a beta cut-off using the from and to squares as identifiers. It also punishes quiet moves that were ordered first but did not cause the desired beta cut-off. The scores are clamped, and are applied so that a smaller bonus is given for expected beta cut-offs, but a larger bonus is given for unexpected beta cut-offs.

The search also makes use of transposition tables with 64-bit unsigned integer Zobrist hashing. Each entry in the transposition table stores the engine's evaluation of the position, allowing the engine to recall an earlier evaluation of the same position without having to re-search it. A zobrist hash of a position is a semi-unique identifier of the position that is calculated by XORing together distinct features of the position (including piece placement). This zobrist hash is used to access the transposition table. If the value extracted from the transposition table is not useful (either because of its low depth or because its score gives no new information), the move stored in the entry is ordered first.

Zobrist hashes stored in a circular buffer are also used for identifying when threefold repetitions occur.

A quiescent search was implemented for all capturing moves. The purpose of the quiescent search is to, starting at the leaf nodes, play out all of the immediate captures. Without this, it is entirely possible that the engine would conclude its search right before losing an important piece (with the losing line being "outside of the horizon," ie. the horizon effect).

The engine has an implementation of Null Move Pruning (NMP). During search, the engine gives the opponent an extra move at reduced depth and a null window search. If our position is so good that the search exceeds beta anyway, then we can say that we've achieved a beta cut-off. It should be noted that NMP should not be done when we are currently in check (we can't pass our turn) or we are in zugswang (where we would actually prefer to pass our turn to keep our advantage but the rules don't allow it). Currently, the implementation does not have any zugswang checks, as there are no promotion rules in Hyper Chess.

### Evaluation
Using bitwise manipulation on bitboards provides a quick and easy way of extracting features out of the position. For instance, the engine identifies immobilized pieces using `kingMoves[immSq] & position[opposingSide]`. The engine also makes use of incremental evaluation techniques, such as maintaining a material and piece square table value instead of recalculating it every time.

Below is a quick list of some of the evaluation heuristics the engine calculates:
- How prone immobilized pieces are to being captured
- The mobility (number of pseudo-legal moves excluding straddlers and the king)
- The number of lines of attack against an immobilized immobilizer

In fact, all three of these heuristics require the use of bitboards.

Piece square tables (PSQT) are used to give a bonus/penalty to the current placement of pieces. These tables act as a guideline for the engine when it cannot see far enough to see the long-term impacts of its arbitrary piece configuration.
