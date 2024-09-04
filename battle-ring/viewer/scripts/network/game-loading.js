
const gameLoadingIndexInput = document.getElementById("game-loading_index");

function prevGame(){
    gameLoadingIndexInput.value = parseInt(gameLoadingIndexInput.value) - 1;
    loadGame();
}

function nextGame(){
    gameLoadingIndexInput.value = parseInt(gameLoadingIndexInput.value) + 1;
    loadGame();
}

function loadGame(){
    // request file
    const xhr = new XMLHttpRequest();

    xhr.onreadystatechange = () => {
        if (xhr.readyState == xhr.DONE){
            // load game
            if (xhr.status == 200)
                prettyLoadLANGame(xhr.responseText);
            else
                console.error("Could not load LAN game", xhr.responseText);
        }
    }

    xhr.open("GET", `game-${gameLoadingIndexInput.value}.txt`);
    xhr.send();
}

//evaluateGame(0);

let candidates = [];
let numberOfMoves = 0;
let numberOfTurns = 0;
let numberOfCaptureMoves = 0;
let numberOfCaptures1 = 0;
let numberOfCaptures2 = 0;
let numberOfCaptures3 = 0;
let numberOfCaptures4 = 0;
let numberOfCaptures5 = 0;
let dbFormat = ``;

function loadLANGame(notation){
    const nSplit = notation.split("\n");
    const fen = nSplit[0].replace("FEN: ", "");
    gameState.loadFEN(fen);

    const wp = nSplit[1].replace("White: ", "");
    const bp = nSplit[2].replace("Black: ", "");

    let moveStr = ``;
    let result;

    for (const uci of notation.split("\n")){

        const allMoves = gameState.board.generateMoves(true);
        numberOfTurns++;
        numberOfMoves += allMoves.length;

        for (const m of allMoves){
            if (m.captures.length > 0)
                numberOfCaptureMoves++;
            if (m.captures.length == 1)
                numberOfCaptures1++;
            else if (m.captures.length == 2)
                numberOfCaptures2++;
            else if (m.captures.length == 3)
                numberOfCaptures3++;
            else if (m.captures.length == 4)
                numberOfCaptures4++;
            else if (m.captures.length == 5)
                numberOfCaptures5++;
        }

        const sq = algebraicToSquare(uci);
        // go through all possible moves
        const moves = gameState.board.generatePieceMoves(sq, gameState.board.squares[sq]);

        if (!isNaN(uci) && uci.length > 0){
            result = uci;
            break;
        }

        for (const m of moves){
            if (m.uci == uci){
                moveStr += " " + getMoveSAN(gameState.board, m);
                gameState.board.makeMove(m);
                break;
            }
        }
    }

    dbFormat += `${fen}\t${wp}\t${bp}\t${result}\t\t${moveStr}\n`;
}

function prettyLoadLANGame(notation){
    const notationElems = notation.split("\n");
    
    const fen = notationElems[0];
    gameState.loadFEN(fen.replace("FEN: ", ""));
    
    for (const uci of notationElems){

        const sq = algebraicToSquare(uci);
        // go through all possible moves
        const moves = gameState.board.generatePieceMoves(sq, gameState.board.squares[sq]);

        for (const m of moves){
            if (m.uci == uci){
                gameState.makeMove(m);
                break;
            }
        }
    }

    // load player names
    const white = notationElems[1].replace("White: ", "");
    const black = notationElems[2].replace("Black: ", "");

    // determine white and black scores
    let resultElem = notationElems[notationElems.length - 2];
    let whiteScore = "-";
    let blackScore = "-";

    if (resultElem == "0"){
        whiteScore = "1/2";
        blackScore = "1/2";
    }else if (resultElem == "-1"){
        whiteScore = "0";
        blackScore = "1";
    }else if (resultElem == "1"){
        whiteScore = "1";
        blackScore = "0";
    }
    
    // display player names and result
    document.getElementById("white_player").innerText = `${white} | ${whiteScore}`;
    document.getElementById("black_player").innerText = `${black} | ${blackScore}`;
}

const materialPoints = [ 0, 0, 300, 900, 400, 1100, 100, 1300 ];

function evaluateGame(index){
    console.log(`Consider game ${index}`);
    const xhr = new XMLHttpRequest();

    xhr.onreadystatechange = () => {
        if (xhr.readyState == 4 && xhr.status == 200){
            loadLANGame(xhr.responseText);
            if (gameState.board.isGameOver() == "#"){
                // after a player delivers checkmate, it's technically the opponent's turn (who has no response).
                // therefore, the winner is whoever's turn it is not.
                const winner = gameState.board.turn == Piece.black ? Piece.white : Piece.black;
                const winnerPerspective = winner == Piece.white ? 1 : -1;

                let materialEval = 0;

                for (const s of gameState.board.squares){
                    const perspective = Piece.getColor(s) == Piece.white ? 1 : -1;
                    materialEval += perspective * materialPoints[Piece.getType(s)];
                }

                if (materialEval * winnerPerspective <= -600 && !gameState.board.isImmobilized(gameState.board.kings[gameState.board.turn == Piece.white ? 0 : 1], gameState.board.turn | Piece.king)){
                    console.log("Candidate!");
                    candidates.push(xhr.responseText);
                }
            }
            evaluateGame(index + 1);
        }
    }

    xhr.open("GET", `game-${index}.txt`);
    xhr.send();
}
