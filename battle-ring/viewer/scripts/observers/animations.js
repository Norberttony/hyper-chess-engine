// handles move animations

var containerElem = document.getElementById("container");

let lastPlayedSAN;

let playerMadeMove = false;
containerElem.addEventListener("madeMove", (event) => {
    const {state, board, san, move} = event.detail;

    // a bad solution to handle moves played by an enemy player (network)
    // generally, I'd prefer if all of the observers everywhere were decoupled completely...
    if (lastPlayedSAN == san) return;

    playerMadeMove = true;
});

let prevMove = gameState.currentMove;
containerElem.addEventListener("movescroll", (event) => {
    const {state, board, pgnMove} = event.detail;

    if (playerMadeMove){
        playerMadeMove = false;
        prevMove = pgnMove;
        return;
    }

    // display the board from just before the move was made.
    let dir = prevMove.isBefore(pgnMove) == 1 ? 1 : -1;
    let lastMadeMove = pgnMove.move;
    if (dir == -1) lastMadeMove = prevMove.move;

    prevMove = pgnMove;

    if (!lastMadeMove) return;

    // now move piece graphically
    let piece;

    if (dir == 1){
        piece = document.getElementById(`${getFileFromSq(lastMadeMove.to)}_${getRankFromSq(lastMadeMove.to)}`);
        setElemLocation(piece, getFileFromSq(lastMadeMove.from), getRankFromSq(lastMadeMove.from));
    }else{
        piece = document.getElementById(`${getFileFromSq(lastMadeMove.from)}_${getRankFromSq(lastMadeMove.from)}`);
        setElemLocation(piece, getFileFromSq(lastMadeMove.to), getRankFromSq(lastMadeMove.to));
    }

    setTimeout(() => {
        if (dir == 1)
            setElemLocation(piece, getFileFromSq(lastMadeMove.to), getRankFromSq(lastMadeMove.to));
        else
            setElemLocation(piece, getFileFromSq(lastMadeMove.from), getRankFromSq(lastMadeMove.from));
    }, 1);
});
