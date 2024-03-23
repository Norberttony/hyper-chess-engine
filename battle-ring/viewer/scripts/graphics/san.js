
function getMoveSAN(board, move, moveList = board.generateMoves()){
    let SAN;

    /* collects information on move collision ambiguity */
    let sameMove = false;
    let sameFile = false;
    let sameRank = false;
    for (const other of moveList){
        if (!(move.from == other.from) && move.to == other.to && Piece.ofType(board.squares[move.from], board.squares[other.from])){
            // oh no, the move is ambiguous!
            sameMove = true;

            // do we need to specify the rank (first & foremost?)
            if (squareToAlgebraicRank(move.from) == squareToAlgebraicRank(other.from))
                sameRank = true;
            
            // what about the file
            if (squareToAlgebraicFile(move.from) == squareToAlgebraicFile(other.from))
                sameFile = true;
        }
    }

    let movingPieceType = Piece.getType(board.squares[move.from]);

    // using information from move collision ambiguity, determine the resolving square
    let resolvedSquare = "";
    if (sameMove){
        if (sameRank || (!sameRank && !sameFile))
            resolvedSquare += squareToAlgebraicFile(move.from);
        if (sameFile)
            resolvedSquare += squareToAlgebraicRank(move.from);
    }
    if (Piece.ofType(board.squares[move.from], Piece.pawn) && board.squares[move.to]){
        resolvedSquare = squareToAlgebraicFile(move.from);
    }

    SAN = `${PieceASCII[movingPieceType]}${resolvedSquare}${move.captures.length > 0 ? "x": ""}${squareToAlgebraic(move.to)}`;

    board.makeMove(move);

    // is game over?
    let result = board.isGameOver();
    if (result == "#"){
        SAN += result;
    }else{
        // does this move threaten to take the king on the next turn?
        board.nextTurn();
        const moves = board.generateMoves(false);
        board.nextTurn();

        let isCheck = false;
        for (const m of moves){
            for (const c of m.captures){
                if (Piece.ofType(c.captured, Piece.king)){
                    isCheck = true;
                    break;
                }
            }
            if (isCheck)
                break;
        }
        if (isCheck)
            SAN += "+";
    }

    board.unmakeMove(move);

    return SAN;
}
