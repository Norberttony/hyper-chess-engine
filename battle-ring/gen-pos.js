
import fs from "fs";

import { Piece } from "./game-logic/piece.js";
import { Board, StartingFEN } from "./game-logic/game.js";

const contents = fs.readFileSync("./pgn.txt").toString();

// returns true if the position is quiet (no capture moves) AND if the material is equal AND if the
// pseudolegal moves are exactly the same as the legal moves (no pins, checks, king attacks)
// Equal material occurs when both opponents have the same number of each piece type.
function isPositionQuietAndEqual(board){
    // 8 entries, first one is empty, the next index relevant piece type
    let pieceCounts = [0, 0, 0, 0, 0, 0, 0, 0];
    let zeroCounts = 8; // the number of zeroes
    for (let s = 0; s < 64; s++){
        const val = board.squares[s];

        // don't count empty squares
        if (val == 0)
            continue;

        // if this piece is immobilized, that's not a quiet position!
        if (board.isImmobilized(s, val))
            return false;

        const type = Piece.getType(val);
        const perspective = Piece.getColor(val) == Piece.white ? 1 : -1;

        // will now be imbalanced
        if (pieceCounts[type] == 0)
            zeroCounts--;

        pieceCounts[type] += perspective;

        // rebalanced it!
        if (pieceCounts[type] == 0)
            zeroCounts++;
    }

    if (zeroCounts != 8)
        return false;

    // now test for a quiet position
    for (const m of board.generateMoves(false)){
        // this indicates that there is some kind of pin or square-limiting attack on the king
        // or a check... best to avoid these types of positions.
        if (!board.isMoveLegal(m))
            return false;

        if (m.captures.length > 0)
            return false;
    }

    // time to test for the other side!!!
    board.nextTurn();
    // now test for a quiet position
    for (const m of board.generateMoves(false)){
        // this indicates that there is some kind of pin or square-limiting attack on the king
        // or a check... best to avoid these types of positions.
        if (!board.isMoveLegal(m)){
            board.nextTurn();
            return false;
        }

        if (m.captures.length > 0){
            board.nextTurn();
            return false;
        }
    }
    board.nextTurn();

    // position must be quiet!
    return true;
}

function recordFENSample(potentialFENs, maxSamples){
    const samplesAmt = Math.max(potentialFENs.length, maxSamples);
    const samples = [];
    let rand;
    while (samples.length != samplesAmt && potentialFENs.length > 0){
        rand = randomInteger(0, potentialFENs.length);
        const val = potentialFENs[rand];
        if (samples.indexOf(val) == -1){
            samples.push(val);
        }
        potentialFENs.splice(rand, 1);
    }

    for (const fen of samples){
        FENs += `${fen}\n`;
        console.log(fen);
    }

    return samples;
}


const game = new Board();

let FENs = "";

let start = contents.indexOf("[");
let end = contents.indexOf("[", contents.indexOf("1."));
let prog = 0;
while (start != -1 && end != -1){
    console.log(prog++);

    game.loadFEN(StartingFEN);

    let pgn = contents.substring(start, end);

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

    let potentialFENs = [];

    // load the pgn
    const moves = pgn.split(" ");
    for (let i = 0; i < moves.length; i++){
        const m1 = game.getMoveOfSAN(moves[i])
        if (m1){
            game.makeMove(m1);
        }

        if (isPositionQuietAndEqual(game)){
            potentialFENs.push(game.getFEN());
        }
    }

    recordFENSample(potentialFENs, Math.ceil(moves.length / 10));

    start = contents.indexOf("[", contents.indexOf("1.", end));
    end = contents.indexOf("[", contents.indexOf("1.", start));
}

for (let i = 0; i < 1000; i++){
    console.log(i);
    // can read the contents of any file with LAN (long algebraic notation) moves.
    const contents = fs.readFileSync(`./debug/game-${i}.txt`).toString();

    game.loadFEN(StartingFEN);

    const moves = contents.split("\n");
    let potentialFENs = [];
    for (let j = 3; j < moves.length - 1; j++){
        if (!moves[j])
            continue;
        game.playLANMove(moves[j].trim());

        if (isPositionQuietAndEqual(game)){
            potentialFENs.push(game.getFEN());
        }
    }

    recordFENSample(potentialFENs, Math.ceil(moves.length / 10));
    
}

fs.appendFileSync("./positions.txt", FENs);

function randomInteger(min, max){
    return Math.floor(Math.random() * (max - min)) + min;
}
