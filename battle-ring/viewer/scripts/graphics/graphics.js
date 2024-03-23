let testMove;

let curBoardDisplay;
let lastMoveDisplay;
let isDisplayFlipped = false;

// properly displays all relevant board state information to the user
// takes in:
//  - board: a Board instance (on which lastMove was just played on)
//  - lastMove: the last Move played
//  - flipped: whether or not the board is flipped
//  - container: the game container HTML element
// all of which are optional and default to the current game container stuff
function displayBoard(board = gameState.board, lastMove = gameState.currentMove != gameState.moveRoot ? gameState.currentMove.move : undefined, flipped = isDisplayFlipped, container = gameElem){
    curBoardDisplay = board;
    isDisplayFlipped = flipped;
    lastMoveDisplay = lastMove; // terrible variable.

    setAllPiecesToPool(container);
    setAllMoveHighlightsToPool(container);
    setAllLastMoveHighlightsToPool(container);

    // highlight move from and move to
    if (lastMove){
        getLastMoveHighlightFromPool(lastMove.to % 8, Math.floor(lastMove.to / 8));
        getLastMoveHighlightFromPool(lastMove.from % 8, Math.floor(lastMove.from / 8));
    }

    // display all pieces on the board
    for (let r = 0; r < 8; r++){
        for (let f = 0; f < 8; f++){
            let v = board.squares[r * 8 + f];
            if (v != 0){
                let piece = getPieceFromPool(f, r, Piece.getType(v), Piece.getColor(v));
                container.appendChild(piece);
            }
        }
    }

    // hide promotion...
    hidePromotion();
}

displayBoard();

function setFEN(){
    let fen = fenText.value;
    gameState.loadFEN(fen);
}

function setPGN(){
    let pgn = pgnText.value;
    gameState.loadPGN(pgn);
}

function undoTestMove(){
    if (testMove){
        testMove = undefined;
        displayBoard();
    }
}

// makes a move that only affects the graphical board
function makeMoveGraphically(move){
    let graphicalState = new Board();
    graphicalState.loadFEN(gameState.board.getFEN());
    graphicalState.makeMove(move);
    displayBoard(graphicalState, move);
    graphicalState.nextTurn(); //goofy.
}

// flips the graphical board
function flipBoard(){
    undoTestMove();
    document.getElementById("container").classList.toggle("flipped");
    displayBoard(curBoardDisplay, lastMoveDisplay, !isDisplayFlipped);
}

function setFlip(flip){
    if (flip){
        document.getElementById("container").classList.add("flipped");
        displayBoard(curBoardDisplay, lastMoveDisplay, true);
    }else{
        document.getElementById("container").classList.remove("flipped");
        displayBoard(curBoardDisplay, lastMoveDisplay, false);
    }
}

// prevent focusing on buttons (so that arrow key presses and other things still register on the
// board, even if the user clicks other buttons like "copy PGN")
let buttons = document.getElementsByTagName("button");
for (let i = 0; i < buttons.length; i++){
    buttons[i].onmousedown = (event) => {
        event.preventDefault();
    }
}
