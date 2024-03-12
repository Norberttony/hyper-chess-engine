
const fs = require("fs");

const { Board, StartingFEN } = require("./game-logic/game");

const contents = fs.readFileSync("./pgn.txt").toString();

const game = new Board();

let FENs = "";

let start = contents.indexOf("[");
let end = contents.indexOf("[", contents.indexOf("1."));
while (start != -1 && end != -1){

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

    // load the pgn
    const moves = pgn.split(" ");
    const randomLength = randomInteger(3, Math.min(50, moves.length));
    for (let i = 0; i < randomLength; i += 2){
        const m1 = game.getMoveOfSAN(moves[i])
        if (m1 && i < randomLength){
            game.makeMove(m1);
        }

        const m2 = game.getMoveOfSAN(moves[i + 1]);
        if (m2 && i + 1 < randomLength){
            game.makeMove(m2);
        }
    }

    FENs += `${game.getFEN()}\n`;

    start = contents.indexOf("[", contents.indexOf("1.", end));
    end = contents.indexOf("[", contents.indexOf("1.", start));
}

for (let i = 0; i <= 65; i++){
    // can read the contents of any file with LAN (long algebraic notation) moves.
    const contents = fs.readFileSync(`./debug - Copy (2)/game-${i}.txt`).toString();

    game.loadFEN(StartingFEN);

    const moves = contents.split("\n");
    const randomLength = randomInteger(6, Math.min(50, moves.length - 1));
    const sampleLength = randomInteger(6, randomLength);
    for (let j = 2; j < randomLength; j++){
        game.playLANMove(moves[j].trim());

        if (sampleLength == j){
            FENs += `${game.getFEN()}\n`;
        }
    }

    FENs += `${game.getFEN()}\n`;
}

fs.appendFileSync("./positions.txt", FENs);

function randomInteger(min, max){
    return Math.floor(Math.random() * (max - min)) + min;
}
