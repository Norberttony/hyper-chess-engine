// contains all of the game logic

// this code REPEATEDLY violates the DRY principle. read at your own risk.

if (typeof module !== "undefined"){
    var { algebraicToSquare, squareToAlgebraic, squareToAlgebraicFile, squareToAlgebraicRank, getFileFromSq, getRankFromSq } = require("./coords");
    var { Piece, PieceTypeToFEN, FENToPiece, PieceASCII } = require("./piece");
    ({ Move } = require("./move"));
    var { numSquaresToEdge, dirOffsets } = require("./pre-game");
    var { getMoveSAN } = require("./san");
}

// removes all glyphs from SAN
function removeGlyphs(san){
    san = san.replace(/[#+?!]/g, "");
    return san;
}

const StartingFEN = "unbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNU w 1";

// The Board object contains a game state of the board. Certain moves can be done or undone, but
// they are not stored.
// Note on three fold repetition checking:
// - class Board DOES NOT handle it currently! all of it is done by the GraphicalState (because it
//      is rather slow). I was considering implementing Zobrist hashing for Board to allow really
//      fast searching, albeit occasionally very very slightly inaccurate. That has not been done :)
class Board {
    constructor(){
        // contains the type and color of every piece on all 64 squares (Piece[Color] | Piece[Type])
        this.squares = new Uint8Array(64);

        this.result;
        this.termination;

        this.turn = Piece.white;

        // positions of kings
        this.kings = new Uint8Array(2);

        // positions of coordinators
        this.coordinators = new Uint8Array(2);

        // positions of chameleons
        this.chameleons = new Uint8Array(4);

        // fullmove counter
        this.fullmove = 1;

        this.loadFEN(StartingFEN);
    }
    // returns any unique identifiers to a position (arrangement of pieces, castling rights, en passant, whose turn it is, etc)
    getPosition(){
        let position = "";
        for (let i = 0; i < 64; i++){
            let v = this.squares[i];
            if (v){
                let c = PieceTypeToFEN[Piece.getType(v)];
                if (Piece.ofColor(v, Piece.white)) c = c.toUpperCase();
                position += c;
            }else{
                position += "0";
            }
        }
        position += this.turn;
        return position;
    }
    setResult(result, termination){
        this.result = result;
        this.termination = termination;
        return result;
    }
    // checks if the current player is checkmated... or stalemated...
    isGameOver(){
        if (this.result) return this.result;

        let moves = this.generateMoves();
        
        // yup. this code is not broken. ha ha.
        this.nextTurn();

        // no legal moves?!
        if (moves.length == 0){
            if (this.isAttacked(this.getKingSq())){
                // CHECKMATE!!!
                this.setResult("#", "checkmate");
            }else{
                // stalemate...!
                this.setResult("/", "stalemate");
            }
        }

        // nothing to see here...!
        this.nextTurn();

        return this.result;
    }

    // ========================================= GENERATE MOVES START ========================================= //
    // detects which piece this is, and generates moves for it. Generally used for graphical side of app.
    generatePieceMoves(start, piece){
        let moves = [];

        if (Piece.ofColor(piece, this.turn)){
            switch(Piece.getType(piece)){
                case Piece.straddler:
                    this.generateStraddlerMoves(start, piece, moves);
                    break;
                case Piece.coordinator:
                    this.generateCoordinatorMoves(start, piece, moves);
                    break;
                case Piece.springer:
                    this.generateSpringerMoves(start, piece, moves);
                    break;
                case Piece.retractor:
                    this.generateRetractorMoves(start, piece, moves);
                    break;
                case Piece.immobilizer:
                    this.generateImmobilizerMoves(start, piece, moves);
                    break;
                case Piece.chameleon:
                    this.generateChameleonMoves(start, piece, moves);
                    break;
                case Piece.king:
                    this.generateKingMoves(start, piece, moves);
                    break;
            }
        }

        return this.filterLegalMoves(moves);
    }
    // generates all possible moves for the given turn
    generateMoves(filter = true){
        let moves = [];

        for (let s = 0; s < 64; s++){
            let piece = this.squares[s];

            if (Piece.ofColor(piece, this.turn)){
                switch(Piece.getType(piece)){
                    case Piece.straddler:
                        this.generateStraddlerMoves(s, piece, moves);
                        break;
                    case Piece.coordinator:
                        this.generateCoordinatorMoves(s, piece, moves);
                        break;
                    case Piece.springer:
                        this.generateSpringerMoves(s, piece, moves);
                        break;
                    case Piece.retractor:
                        this.generateRetractorMoves(s, piece, moves);
                        break;
                    case Piece.immobilizer:
                        this.generateImmobilizerMoves(s, piece, moves);
                        break;
                    case Piece.chameleon:
                        this.generateChameleonMoves(s, piece, moves);
                        break;
                    case Piece.king:
                        this.generateKingMoves(s, piece, moves);
                        break;
                }
            }
        }

        if (filter) moves = this.filterLegalMoves(moves);

        return moves;
    }

    // returns true if the given pieceType is within a 1 square manhattan distance to the given sq
    // AND if that piece is of opposite color to the given piece.
    inVicinity(sq, piece, pieceType){
        // go through all 8 directions
        for (let i = 0; i < 8; i++){

            // prevent wrapping around board
            if (numSquaresToEdge[sq][i] == 0)
                continue;

            const target = sq + dirOffsets[i];
            const targetValue = this.squares[target];

            if (Piece.ofType(targetValue, pieceType) && !Piece.ofColor(piece, targetValue)){
                return true;
            }

        }
        return false;
    }

    // returns true if a given piece is within the sphere of influence of an enemy immobilizer.
    isImmobilized(sq, piece){
        return this.inVicinity(sq, piece, Piece.immobilizer);
    }

    // returns true if a given piece is within the sphere of influence of an enemy chameleon.
    isChameleoned(sq, piece){
        return this.inVicinity(sq, piece, Piece.chameleon);
    }

    generateChameleonMoves(start, piece, moves){

        if (this.isImmobilized(start, piece))
            return;

        // for copying coordinator moves
        const enemyCoordSq = this.coordinators[this.turn == Piece.white ? 1 : 0];
        const enemyCoordRank = getRankFromSq(enemyCoordSq);
        const enemyCoordFile = getFileFromSq(enemyCoordSq);

        const kingSq = this.getCurKingSq();
        const kingRank = getRankFromSq(kingSq);
        const kingFile = getFileFromSq(kingSq);

        // for copying king moves
        const coordSq = this.coordinators[this.turn == Piece.white ? 0 : 1];
        const coordRank = getRankFromSq(coordSq);
        const coordFile = getFileFromSq(coordSq);

        const enemyKingSq = this.getEnemyKingSq();
        const enemyKingRank = getRankFromSq(enemyKingSq);
        const enemyKingFile = getFileFromSq(enemyKingSq);

        // determines number of valid directions the piece can go through
        for (let i = 0; i < 8; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){

                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                const captures = [];

                // check coordinator type moves
                // checking for targetValue == 0 is fine because a coordinator cannot make a
                // capturing move that occupies the enemy's square.
                if (targetValue == 0 && kingSq != 255 && enemyCoordSq != 255){
                    const targetRank = getRankFromSq(target);
                    const targetFile = getFileFromSq(target);
                    if (kingRank == enemyCoordRank && targetFile == enemyCoordFile || kingFile == enemyCoordFile && targetRank == enemyCoordRank){
                        captures.push({sq: enemyCoordSq, captured: this.squares[enemyCoordSq]});
                    }
                }

                // check retractor type moves
                if (targetValue == 0 && j == 0){
                    // check backwards
                    let d = (i + 2) % 4;
                    if (i >= 4)
                        d += 4;

                    if (numSquaresToEdge[start][d] > 0){
                        const deathSq = start + dirOffsets[d];
                        const deathVal = this.squares[deathSq];
                        if (deathVal != 0 && !Piece.ofColor(piece, deathVal) && Piece.ofType(deathVal, Piece.retractor)){
                            captures.push({sq: deathSq, captured: deathVal});
                        }
                    }
                }

                // check king type moves
                if (targetValue != 0 && j == 0 && !Piece.ofColor(piece, targetValue) && Piece.ofType(targetValue, Piece.king)){
                    // this would cause problems if the king was not a royal piece. but it is :)
                    moves.push(new Move(target, start, [{sq: target, captured: targetValue}]));
                    break;
                }

                // check springer type moves
                if (targetValue != 0 && !Piece.ofColor(piece, targetValue) && Piece.ofType(targetValue, Piece.springer) && numSquaresToEdge[target][i] > 0 && this.squares[target + dirOffsets[i]] == 0){
                    // there is actually no way for this move to be covered because of the rules :)
                    moves.push(new Move(target + dirOffsets[i], start, [{sq: target, captured: targetValue}]));
                    break;
                }

                // king type moves, where chameleon acts like king and teams up with own coordinator
                if (targetValue == 0 && j == 0 && coordSq != 255){
                    const targetRank = getRankFromSq(target);
                    const targetFile = getFileFromSq(target);

                    if (targetRank == enemyKingRank && coordFile == enemyKingFile || targetFile == enemyKingFile && coordRank == enemyKingRank){
                        captures.push({sq: enemyKingSq, captured: this.squares[enemyKingSq]});
                    }
                }

                // straddler type moves
                if (targetValue == 0 && i < 4){
                    for (let k = -1; k <= 1; k++){
                        const d = (i + 4 + k) % 4;

                        if (numSquaresToEdge[target][d] <= 1)
                            continue;

                        const nextTarget = target + dirOffsets[d];
                        const nextTargetValue = this.squares[nextTarget];

                        if (!Piece.ofColor(piece, nextTargetValue) && Piece.ofType(Piece.straddler, nextTargetValue)){
                            const nextNextTarget = target + 2 * dirOffsets[d];
                            const nextNextTargetValue = this.squares[nextNextTarget];

                            if (Piece.ofColor(piece, nextNextTargetValue) && (Piece.ofType(nextNextTargetValue, Piece.chameleon) || Piece.ofType(nextNextTargetValue, Piece.straddler))){
                                captures.push({sq: nextTarget, captured: nextTargetValue});
                            }
                        }
                    }
                }

                if (targetValue == 0)
                    moves.push(new Move(target, start, captures));
                else
                    break;

            }
        }
    }

    generateImmobilizerMoves(start, piece, moves){
        if (this.isChameleoned(start, piece) || this.isImmobilized(start, piece))
            return;

        let dirStart = 0;
        let dirEnd = 8;
    
        // determines number of valid directions the piece can go through
        for (let i = dirStart; i < dirEnd; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){
                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                if (targetValue == 0){
                    moves.push(new Move(target, start));
                }else{
                    break;
                }
            }
        }
    }

    generateRetractorMoves(start, piece, moves){
        if (this.isImmobilized(start, piece))
            return;

        let dirStart = 0;
        let dirEnd = 8;
    
        // determines number of valid directions the piece can go through
        for (let i = dirStart; i < dirEnd; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){

                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                let captures = [];
                if (targetValue == 0 && j == 0){
                    // check backwards
                    let d = (i + 2) % 4;
                    if (i >= 4)
                        d += 4;

                    if (numSquaresToEdge[start][d] > 0){
                        const deathSq = start + dirOffsets[d];
                        const deathVal = this.squares[deathSq];
                        if (deathVal != 0 && !Piece.ofColor(piece, deathVal)){
                            captures.push({sq: deathSq, captured: deathVal});
                        }
                    }
                }
                
                if (targetValue != 0)
                    break;

                moves.push(new Move(target, start, captures));

            }
        }
    }

    generateSpringerMoves(start, piece, moves){
        if (this.isImmobilized(start, piece))
            return;

        // moves like a chess queen
        for (let i = 0; i < 8; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){

                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                if (targetValue != 0){

                    if (Piece.ofColor(piece, targetValue))
                        break;

                    if (numSquaresToEdge[target][i] == 0)
                        continue;

                    const nextTarget = target + dirOffsets[i];
                    const nextTargetValue = this.squares[nextTarget];

                    if (nextTargetValue == 0){
                        // jump over piece
                        moves.push(new Move(nextTarget, start, [{sq: target, captured: targetValue}]));
                    }
                    break;

                }else{
                    moves.push(new Move(target, start));
                }

            }
        }
    }

    generateCoordinatorMoves(start, piece, moves){
        if (this.isImmobilized(start, piece))
            return;

        let dirStart = 0;
        let dirEnd = 8;

        // a chameleon cannot team up with another chameleon to capture the king.
        // that would make two chameleons much too powerful :)
        const considerSquares = [
            this.getCurKingSq(),
            this.chameleons[this.turn == Piece.white ? 0 : 2],
            this.chameleons[this.turn == Piece.white ? 1 : 3]
        ].filter((val => val != 255));

        const ranks = considerSquares.map((val => getRankFromSq(val)));
        const files = considerSquares.map((val => getFileFromSq(val)));
    
        // determines number of valid directions the piece can go through
        for (let i = dirStart; i < dirEnd; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){

                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                if (targetValue != 0)
                    break;

                const rank = getRankFromSq(target);
                const file = getFileFromSq(target);

                let captures = [];
                for (let s = 0; s < considerSquares.length; s++){
                    const sqRank = ranks[s];
                    const sqFile = files[s];
                    if (sqRank != rank && sqFile != file){
                        // death squares are formed
                        const death1 = rank * 8 + sqFile;
                        const death1Value = this.squares[death1];
                        const death2 = sqRank * 8 + file;
                        const death2Value = this.squares[death2];

                        if (death1Value && !Piece.ofColor(piece, death1Value)){
                            // if chameleon, must be capturing a king
                            if (s == 0 || s > 0 && Piece.ofType(death1Value, Piece.king))
                                captures.push({sq: death1, captured: death1Value});
                        }
                        if (death2Value && !Piece.ofColor(piece, death2Value)){
                            // if chameleon, must be capturing a king
                            if (s == 0 || s > 0 && Piece.ofType(death2Value, Piece.king))
                                captures.push({sq: death2, captured: death2Value});
                        }
                    }
                }

                moves.push(new Move(target, start, captures));
            }
        }
    }

    generateStraddlerMoves(start, piece, moves){
        if (this.isImmobilized(start, piece))
            return;

        // determines number of valid directions the piece can go through
        let canMoveForward = false;
        for (let i = 0; i < 4; i++){
            // goes through all squares until it hits an edge
            for (let j = 0; j < numSquaresToEdge[start][i]; j++){

                const target = start + dirOffsets[i] * (j + 1);
                const targetValue = this.squares[target];

                if (targetValue != 0)
                    break;

                // straddler can now capture a piece ahead of it, to its left, or to its right (relative to movement)
                const captures = [];
                for (let k = -1; k <= 1; k++){
                    const d = (i + 4 + k) % 4;

                    const nextTarget = target + dirOffsets[d];
                    const nextTargetValue = this.squares[nextTarget];

                    if (numSquaresToEdge[target][d] <= 1){
                        if (nextTargetValue == 0)
                            canMoveForward = true;
                        continue;
                    }

                    const nextNextTarget = target + dirOffsets[d] * 2;
                    const nextNextTargetValue = this.squares[nextNextTarget];

                    // must have a center target and a straddler on the other side for the...
                    // CUSTODIAN CAPTURE.
                    // If any of the encompassing pieces are chameleons, the captured piece must also be a straddler.
                    // must be some of the ugliest code in the whole while world :)
                    const chameleonCapt = Piece.ofType(nextNextTargetValue, Piece.chameleon);
                    const canCapture = chameleonCapt && Piece.ofType(nextTargetValue, Piece.straddler) || !chameleonCapt && Piece.ofType(nextNextTargetValue, Piece.straddler);
                    if (nextTargetValue != 0 && !Piece.ofColor(piece, nextTargetValue) && Piece.ofColor(nextNextTargetValue, piece) && canCapture){
                        captures.push({sq: nextTarget, captured: nextTargetValue});
                    }else if (nextTargetValue == 0 && k == 0){
                        canMoveForward = true;
                    }
                }

                moves.push(new Move(target, start, captures));

                if (!canMoveForward){
                    break;
                }
            }
        }

        return moves;
    }

    // generates moves for a king
    generateKingMoves(start, piece, moves){
        if (this.isImmobilized(start, piece))
            return;

        // king can team up with its own coordinator to create death squares
        const coordSq = this.getCurCoordSq();
        const coordRank = getRankFromSq(coordSq);
        const coordFile = getFileFromSq(coordSq);

        // see if king can team up with chameleon and capture an enemy coordinator!
        const enemyCoordSq = this.coordinators[this.turn == Piece.white ? 1 : 0];
        const enemyCoordRank = getRankFromSq(enemyCoordSq);
        const enemyCoordFile = getFileFromSq(enemyCoordSq);

        let toEdge = numSquaresToEdge[start];

        // check all directions
        for (let i = 0; i < dirOffsets.length; i++){
            if (toEdge[i] > 0){

                const target = start + dirOffsets[i];
                const targetValue = this.squares[target];

                if (targetValue != 0 && Piece.ofColor(piece, targetValue)){
                    continue;
                }

                const rank = getRankFromSq(target);
                const file = getFileFromSq(target);

                let captures = [];
                if (coordSq != 255 && coordRank != rank && coordFile != file){
                    // death squares are formed
                    const death1 = rank * 8 + coordFile;
                    const death1Value = this.squares[death1];
                    const death2 = coordRank * 8 + file;
                    const death2Value = this.squares[death2];

                    if (death1Value && !Piece.ofColor(piece, death1Value)){
                        captures.push({sq: death1, captured: death1Value});
                    }
                    if (death2Value && !Piece.ofColor(piece, death2Value)){
                        captures.push({sq: death2, captured: death2Value});
                    }
                }

                // test for king-chameleon death squares
                if (enemyCoordSq != 255){
                    const thisRank = getRankFromSq(target);
                    const thisFile = getFileFromSq(target);

                    const chamStart = this.turn == Piece.white ? 0 : 2;
                    const chamEnd = this.turn == Piece.white ? 2 : 4;
                    for (let i = chamStart; i < chamEnd; i++){
                        if (this.chameleons[i] == 255)
                            break;

                        const rank = getRankFromSq(this.chameleons[i]);
                        const file = getFileFromSq(this.chameleons[i]);

                        // forms death squares, but only against the enemy coordinator.
                        // and there's always only one enemy coordinator.
                        // of course, the king isn't actually forming chameleon death squares if the two are aligned (rank/file)
                        if (rank == enemyCoordRank && thisFile == enemyCoordFile && thisFile != file && thisRank != rank){
                            captures.push({sq: enemyCoordSq, captured: this.squares[enemyCoordSq]});
                            break;
                        }
                        if (file == enemyCoordFile && thisRank == enemyCoordRank && thisFile != file && thisRank != rank){
                            captures.push({sq: enemyCoordSq, captured: this.squares[enemyCoordSq]});
                            break;
                        }
                    }
                }

                if (targetValue != 0 && !Piece.ofColor(piece, targetValue)){
                    captures.push({sq: target, captured: targetValue});
                }

                moves.push(new Move(target, start, captures));
            }
        }
    }
    // ========================================= GENERATE MOVES END ========================================= //

    // takes in a list of moves, and gives a list of all legal moves
    filterLegalMoves(moves){
        for (let i = 0; i < moves.length; i++){
            if (!this.isMoveLegal(moves[i])){
                moves.splice(i, 1);
                i--;
            }
        }
        return moves;
    }
    // retrieves the current player's king's square
    getKingSq(){
        return this.turn == Piece.black ? this.kings[0] : this.kings[1];
    }
    // actually retrieves the current player's king's square
    getCurKingSq(){
        return this.turn == Piece.white ? this.kings[0] : this.kings[1];
    }
    getEnemyKingSq(){
        return this.turn == Piece.white ? this.kings[1] : this.kings[0];
    }
    // retrieves the current player's coordinator's square
    getCurCoordSq(){
        return this.turn == Piece.white ? this.coordinators[0] : this.coordinators[1];
    }
    // checks if a move is legal
    isMoveLegal(move){
        this.makeMove(move);

        // if the move causes the current king to stay in check, then it can't be legal
        let kingSq = this.getKingSq();
        let attacksKing = this.isAttacked(kingSq);

        this.unmakeMove(move);

        return !attacksKing;
    }
    // checks if a certain square is attacked by the side to play
    isAttacked(sq){

        // go through every move
        let test = this.generateMoves(false);
        for (const m of test){
            for (const c of m.captures){
                if (c.sq == sq)
                    return true;
            }
        }

        return false;
    }
    // performs a move on the board. this method assumes that the given move is LEGAL
    // if the move is not legal, then some funny behavior could occur.
    makeMove(move){

        // update king locations if necessary.
        if (Piece.ofType(this.squares[move.from], Piece.king)){
            if (this.turn == Piece.black){
                this.kings[1] = move.to;
            }else{
                this.kings[0] = move.to;
            }
        }

        // update coordinator locations if necessary
        if (Piece.ofType(this.squares[move.from], Piece.coordinator)){
            if (this.turn == Piece.black){
                this.coordinators[1] = move.to;
            }else{
                this.coordinators[0] = move.to;
            }
        }

        // update chameleon locations if necessary
        if (Piece.ofType(this.squares[move.from], Piece.chameleon)){
            if (this.turn == Piece.black){
                if (this.chameleons[2] == move.from){
                    this.chameleons[2] = move.to;
                }else{
                    this.chameleons[3] = move.to;
                }
            }else{
                if (this.chameleons[0] == move.from){
                    this.chameleons[0] = move.to;
                }else{
                    this.chameleons[1] = move.to;
                }
            }
        }

        // go through all of the captures
        for (const {sq, captured} of move.captures){
            this.squares[sq] = 0;
            if (Piece.ofType(captured, Piece.coordinator)){
                // remove coordinator from list
                this.coordinators[Piece.getColor(captured) / 8 - 1] = 255;
            }else if (Piece.ofType(captured, Piece.chameleon)){
                // remove chameleon from list
                if (this.turn == Piece.white){
                    if (this.chameleons[2] == sq){
                        this.chameleons[2] = this.chameleons[3];
                    }
                    this.chameleons[3] = 255;
                }else{
                    if (this.chameleons[0] == sq){
                        this.chameleons[0] = this.chameleons[1];
                    }
                    this.chameleons[1] = 255;
                }
            }else if (Piece.ofType(captured, Piece.king)){
                if (this.turn == Piece.white){
                    this.kings[1] = 255;
                }else{
                    this.kings[0] = 255;
                }
            }
        }

        // moves the piece to its designated square, leaving nothing behind
        this.squares[move.to] = this.squares[move.from];
        this.squares[move.from] = 0;
        
        // fullmove
        if (Piece.ofColor(this.turn, Piece.black)){
            if (this.fullmove != "-") this.fullmove++;
        }

        // set turn
        this.nextTurn();
    }
    // un-does a move on the board (make sure that the move being undone is the most recent made move)
    unmakeMove(move){
        // unmove the piece and uncapture whatever it captured.
        this.squares[move.from] = this.squares[move.to];
        this.squares[move.to] = 0;
        for (const {sq, captured} of move.captures){
            this.squares[sq] = captured;
            if (Piece.ofType(captured, Piece.coordinator)){
                // add coordinator back to list
                this.coordinators[Piece.getColor(captured) / 8 - 1] = sq;
            }else if (Piece.ofType(captured, Piece.chameleon)){
                // remove chameleon from list
                if (this.turn == Piece.black){
                    if (this.chameleons[2] == 255){
                        this.chameleons[2] = sq;
                    }else{
                        this.chameleons[3] = sq;
                    }
                }else{
                    if (this.chameleons[0] == 255){
                        this.chameleons[0] = sq;
                    }else{
                        this.chameleons[1] = sq;
                    }
                }
            }else if (Piece.ofType(captured, Piece.king)){
                if (this.turn == Piece.black){
                    this.kings[1] = sq;
                }else{
                    this.kings[0] = sq;
                }
            }
        }

        // set turn
        this.nextTurn();

        // update king locations if necessary
        if (Piece.ofType(this.squares[move.from], Piece.king))
            this.kings[this.turn == Piece.black ? 1 : 0] = move.from;

        // update coordinator locations if necessary
        if (Piece.ofType(this.squares[move.from], Piece.coordinator))
            this.coordinators[this.turn == Piece.black ? 1 : 0] = move.from;

        // update chameleon locations if necessary
        if (Piece.ofType(this.squares[move.from], Piece.chameleon)){
            if (this.turn == Piece.black){
                if (this.chameleons[2] == move.to){
                    this.chameleons[2] = move.from;
                }else{
                    this.chameleons[3] = move.from;
                }
            }else{
                if (this.chameleons[0] == move.to){
                    this.chameleons[0] = move.from;
                }else{
                    this.chameleons[1] = move.from;
                }
            }
        }

        // fullmove
        if (Piece.ofColor(this.turn, Piece.black) && this.fullmove != "-"){
            this.fullmove--;
        }

        // removes any stored result
        delete this.result;
        delete this.termination;
    }
    // ==== END MOVE GENERATION AND CHECKING ==== //

    // ==== STATE UPDATES ==== //
    // gets move given SAN
    getMoveOfSAN(san){
        if (!san) return;

        const moves = this.generateMoves(true);

        //const start = performance.now();

        for (const m of moves){
            const SAN = getMoveSAN(this, m, moves);
            if (removeGlyphs(SAN) == removeGlyphs(san)){
                //const end = performance.now();
                //console.log(end - start);
                return m;
            }
        }

        return;
    }
    
    nextTurn(){
        this.turn = this.turn == Piece.white ? Piece.black : Piece.white;
    }

    // loads a FEN into this board state
    loadFEN(fen){
        // clear board first
        this.squares = new Uint8Array(64);
        delete this.result;
        this.coordinators[0] = 255;
        this.coordinators[1] = 255;
        this.chameleons[0] = 255;
        this.chameleons[1] = 255;
        this.chameleons[2] = 255;
        this.chameleons[3] = 255;
        this.kings[0] = 255;
        this.kings[1] = 255;

        // 0 = board state
        // 1 = turn
        // 2 = fullmove number (number of fullmoves, starts at 1, incremented after black's move)
        let segments = fen.split(" ");

        // rewrite board state
        let state = segments[0];
        let f = 0;
        let r = 7;
        for (let i = 0; i < state.length; i++){
            let c = state[i];
            
            if (c == "/"){
                r--;
                f = 0;
            }else if (FENToPiece[c]){
                let piece = FENToPiece[c];
                let sq = f + r * 8;
                this.squares[sq] = piece;
                f++;

                // if the piece is a king, record it
                if (Piece.ofType(piece, Piece.king)){
                    Piece.ofColor(piece, Piece.white) ? this.kings[0] = sq : this.kings[1] = sq;
                }

                // if the piece is a coordinator, record it
                if (Piece.ofType(piece, Piece.coordinator)){
                    Piece.ofColor(piece, Piece.white) ? this.coordinators[0] = sq : this.coordinators[1] = sq;
                }

                if (Piece.ofType(piece, Piece.chameleon)){
                    if (Piece.ofColor(piece, Piece.white)){
                        if (this.chameleons[0] == 255){
                            this.chameleons[0] = sq;
                        }else{
                            this.chameleons[1] = sq;
                        }
                    }else{
                        if (this.chameleons[2] == 255){
                            this.chameleons[2] = sq;
                        }else{
                            this.chameleons[3] = sq;
                        }
                    }
                }
            }else{
                f += parseInt(c);
            }
        }

        // set proper turn
        if (segments[1].toLowerCase() == "w"){
            this.turn = Piece.white;
        }else{
            this.turn = Piece.black;
        }

        // fullmove clock
        this.fullmove = parseInt(segments[2]);
        if (isNaN(this.fullmove))
            this.fullmove = "-";
    }

    getFEN(){
        let FEN = "";

        // write board state into FEN
        for (let r = 7; r >= 0; r--){
            let empty = 0;
            for (let f = 0; f < 8; f++){
                let v = this.squares[f + r * 8];
                if (v){
                    if (empty) FEN += empty;
                    empty = 0;

                    let pieceFEN = PieceTypeToFEN[Piece.getType(v)];
                    FEN += Piece.ofColor(v, Piece.white) ? pieceFEN.toUpperCase() : pieceFEN.toLowerCase();
                }else{
                    empty++;
                }
            }
            if (empty) FEN += empty;
            FEN += "/";
        }
        FEN = FEN.substring(0, FEN.length - 1);

        // set proper turn
        let turn = Piece.ofColor(this.turn, Piece.white) ? "w" : "b";

        FEN += ` ${turn} ${this.fullmove}`;

        return FEN;
    }
    // ==== END STATE UPDATES ==== //

    playLANMove(LAN){
        const moves = this.generateMoves(true);

        for (const m of moves){
            if (m.uci == LAN){
                this.makeMove(m);
                return m;
            }
        }
    }

    getLANMove(LAN){
        const moves = this.generateMoves(true);

        for (const m of moves){
            if (m.uci == LAN){
                return m;
            }
        }
    }
}

if (typeof(exports) !== "undefined")
    module.exports = { Board, StartingFEN };
