
const socket = io();

socket.on("newgame", (fen) => {
    gameState.loadFEN(fen);
});

socket.on("move", (lan) => {
    const move = gameState.board.getLANMove(lan);
    gameState.makeMove(move);
});

let userSide = Piece.white;
containerElem.addEventListener("madeMove", (event) => {
    const {state, board, san, move, pgnMove} = event.detail;

    if (board.turn != userSide && pgnMove.prev.next.length == 1){
        lastPlayedSAN = san;
        socket.emit("makemove", move.uci);
    }
});
