var pgnText = document.getElementById("pgnText");
var fenText = document.getElementById("fenText");

// Graphical state connects any graphical interfaces or inputs to the Board object.
// It does this by dispatching various events onto the main container element.
class GraphicalState {
    constructor(containerElem){
        // containerElem is used to dispatch relevant events to the main game container
        this.containerElem = containerElem;

        this.board = new Board();

        this.positions = {};

        this.moveRoot = new PGN_Move();

        this.pgnData = new PGNData(this.moveRoot);

        // the move currently played out on the board
        this.currentMove = this.moveRoot;

        // currently user can make moves for both sides
        this.allowedSides = {
            [Piece.white]: true,
            [Piece.black]: true
        };
    }

    // where move is PGN_Move
    setMove(move){

        // go back to the first move
        while (this.previousMove()){}

        // now go forward to the given move
        for (const v of move.location){
            this.nextMove(v);
        }

        this.graphicsUpdate();
    }

    graphicsUpdate(){
        displayBoard();
        this.dispatchEvent("movescroll", {state: this, board: this.board, pgnMove: this.currentMove});
    }

    nextMove(variation = 0){
        const move = this.currentMove.next[variation];
        if (move){
            this.board.makeMove(move.move);

            // position reoccurs
            this.positions[this.board.getPosition()]++;

            this.currentMove = move;
            return true;
        }
        return false;
    }

    previousMove(){
        if (this.currentMove.prev){
            // position "unoccurs"
            this.positions[this.board.getPosition()]--;

            this.board.unmakeMove(this.currentMove.move);
            this.currentMove = this.currentMove.prev;
            return true;
        }
        return false;
    }

    dispatchEvent(name, detail){
        this.containerElem.dispatchEvent(new CustomEvent(name, {detail}));
    }

    // assumes move is legal
    makeMove(move, dispatchNonPGNEvents = true){
        let SAN = getMoveSAN(this.board, move);

        let pgnMove;
        
        // search for an existing pgnMove
        for (const m of this.currentMove.next){
            if (m.san == SAN){
                pgnMove = m;
                break;
            }
        }
        
        // create new pgnMove
        if (!pgnMove){
            pgnMove = new PGN_Move(move);
            pgnMove.san = SAN;

            pgnMove.attachTo(this.currentMove);
        }

        this.currentMove = pgnMove;

        this.board.makeMove(move);

        if (dispatchNonPGNEvents)
            displayBoard(this.board, move);

        if (dispatchNonPGNEvents)
            this.dispatchEvent("madeMove", {state: this, board: this.board, san: SAN, move: move, pgnMove: pgnMove});

        this.dispatchEvent("pgnMadeMove", {state: this, board: this.board, san: SAN, move: move, pgnMove: pgnMove});

        // let's indicate a move scroll
        if (dispatchNonPGNEvents)
            this.dispatchEvent("movescroll", {state: this, board: this.board, pgnMove: pgnMove});

        // keep track of repeated positions for three fold repetition
        let pos = this.board.getPosition();
        if (this.positions[pos]) this.positions[pos]++;
        else                     this.positions[pos] = 1;

        // the only issue is that this does not handle board (if moved to its latest state)
        if (this.positions[pos] >= 3)
            this.board.setResult("/", "three-fold repetition");

        // check and dispatch event for any results
        this.board.isGameOver();
        if (this.board.result)
            this.dispatchEvent("result", {state: this, board: this.board});
    }

    loadFEN(fen){
        this.positions = {};
        
        this.board.loadFEN(fen);
        
        // just get rid of everything after move root and have gc handle it
        this.currentMove = this.moveRoot;
        this.moveRoot.next = [];

        displayBoard();
        this.dispatchEvent("loadedFEN", {state: this, fen});
    }

    loadPGN(pgn){
        // check if we have to load from position
        let fen = StartingFEN;
        const headers = extractHeaders(pgn);
        if (headers.Variant == "From Position"){
            fen = headers.FEN;
        }
        this.loadFEN(fen);

        // remove headers
        pgn = pgn.replace(/\[.+?\]\s*/g, "");

        // remove any comments
        pgn = pgn.replace(/\{.+?\}\s*/g, "");

        // remove full move counters
        pgn = pgn.replace(/[0-9]+[\.]+/g, "");

        // add a space before and after parentheses
        pgn = pgn.replace(/\(/g, " ( ").replace(/\)/g, " ) ");

        // make sure there is one space between each move
        pgn = pgn.replace(/\s+/g, " ");
        pgn = pgn.trim();

        // start reading san
        let pgnSplit = pgn.split(" ");
        this.readVariation(pgnSplit, 0);
        displayBoard();
    }

    readVariation(pgnSplit, start){

        let toUndo = 0;

        for (let i = start; i < pgnSplit.length; i++){
            
            const pgn = pgnSplit[i];

            if (pgn == "("){

                this.previousMove();

                // start a variation!
                i = this.readVariation(pgnSplit, i + 1);

                // continue with main variation
                this.nextMove(0);

            }else if (pgn == ")"){

                for (let j = 0; j < toUndo; j++){
                    this.previousMove();
                }

                return i;
            }else{
                const move = this.board.getMoveOfSAN(pgn);
                if (move){
                    this.makeMove(move, false);
                    toUndo++;
                }
            }

        }

    }

    // returns true if the given piece at this square can move, and false if it cannot
    // this depends on multiplayer rules (aka, this user cannot move any white/black pieces)
    // BUT this function will NOT ENFORCE the rules for makeMove! it's good for checking if the
    // user can make a test move given the square
    canMove(square){
        const piece = this.board.squares[square];
        
        // if user not allowed to make move for this side
        if (!this.allowedSides[this.board.turn])
            return false;
        
        // user cannot make a second test move
        if (testMove)
            return false;

        // cannot move a piece if it's not that side to play
        if (!Piece.ofColor(piece, this.board.turn))
            return false;

        // cannot move a piece if the result is determined
        if (this.board.result)
            return false;

        // otherwise we're good :)
        return true;
    }

    // determine whether current user permissions indicate spectating
    isSpectator(){
        return !(this.allowedSides[Piece.white] || this.allowedSides[Piece.black]);
    }

    setSide(side){
        this.allowedSides[Piece.white] = false;
        this.allowedSides[Piece.black] = false;

        if (side)
            this.allowedSides[side] = true;
    }
}

window.gameState = new GraphicalState(document.getElementById("container"));
