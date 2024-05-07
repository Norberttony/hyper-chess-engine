// create a web server so people can join and watch the games!
const express = require("express");
const http = require("http");

const app = express();
const server = http.createServer(app);

const io = require("socket.io")(server);

app.use(express.static(__dirname + "/viewer"));
app.use(express.static(__dirname + "/debug"));

app.use(express.json());

app.get("/", (req, res) => {
    res.sendFile("index.html");
});

const sockets = [];

const moves = [];

io.on("connection", (socket) => {

    sockets.push(socket);

    for (const m of moves){
        socket.emit("move", m);
    }

    //socket.on("makemove", socketMakeMove);

});

server.listen(8000);
console.log("Listening to port 8000");


const fs = require("fs");

const { MatchHandler } = require("./match-handler");
const { Engine } = require("./engine");

const { getAllPositions } = require("./fetch-pos");

const botsDir = "./bots/";
const debugDir = "./debug/";

const positions = getAllPositions();

// search for all .exe files (engines)
const engines = [];

fs.readdirSync(botsDir).forEach(file => {
    if (file.endsWith(".exe")){
        // valid!
        const engine = new Engine(file.replace(".exe", ""), `${botsDir}${file}`);
        engines.push(engine);
        console.log(engine);
    }
});

function playGame(white, black, index, fen, handler){
    // create a new match between white and black
    return new MatchHandler(white, black, index, fen, handler);
}

// user plays against engine
/*
console.log("Now playing against", engines[0]);
const engineProc = engines[0].createProcess(playerVsEngineHandler);

engineProc.start();
engineProc.write("");   // fen
engineProc.write("w");  // side to play

function socketMakeMove(move){
    console.log("Received move", move);
    engineProc.write(move);
    moves.push(move);
}

function playerVsEngineHandler(engine, data){
    console.log(data);

    const lines = data.split("\n");

    for (const l of lines){
        const cmds = l.trim().split(" ");

        switch(cmds[0]){
            case "makemove":
                moves.push(cmds[1]);
                for (const socket of sockets)
                    socket.emit("move", cmds[1]);
                console.log(cmds[1]);
                break;
        }
    }
}
*/

// play many games

(async () => {
    let e1Wins = 0;
    let e2Wins = 0;
    let draws = 0;
    let whiteWins = 0;
    let blackWins = 0;

    // read from log file to determine starting point and wins
    const logFileContent = fs.readFileSync(`${debugDir}_log.txt`).toString();
    const logFileLines = logFileContent.split("\n");

    logFileLines.splice(logFileLines.length - 1, 1);

    if (logFileLines.length >= 2){
        const logFileH2H = logFileLines[logFileLines.length - 2].split(" ");
        const logFileWB = logFileLines[logFileLines.length - 1].split(" ");

        e1Wins = parseInt(logFileH2H[1]);
        e2Wins = parseInt(logFileH2H[5]);
        draws = parseInt(logFileH2H[3]);
        whiteWins = parseInt(logFileWB[1]);
        blackWins = parseInt(logFileWB[5]);
    }

    let gameIndex = (e1Wins + draws + e2Wins) / 2;
    const threads = 2;

    // records the result of a finished match
    function recordResult(matchHandler, reverse = false){
        let result = matchHandler.result;

        // determine wins for white, black, or draws
        if (result == 1)
            whiteWins++;
        else if (result == -1)
            blackWins++;
        else if (result == 0)
            draws++;

        // determine wins for engines
        if (reverse)
            result *= -1;

        if (result == 1){
            e1Wins++;
        }else if (result == -1){
            e2Wins++;
        }
    }

    let startDouble = () => {
        if (positions.length == 0){
            console.log("out of positions!");
            return;
        }

        let myIndex = gameIndex++;

        if (myIndex > 1000)
            return;

        console.log("Starting double", myIndex * 2);
        const chosen = positions[Math.floor(Math.random() * positions.length)];
        console.log("Chose position", chosen);

        // remove position from samples
        positions.splice(positions.indexOf(chosen), 1);


        playGame(engines[0], engines[1], myIndex * 2, chosen.trim(), (matchHandler) => {
            recordResult(matchHandler, false);

            if (matchHandler.result == -2)
                return;

            playGame(engines[1], engines[0], myIndex * 2 + 1, chosen.trim(), (matchHandler) => {
                recordResult(matchHandler, true);

                if (matchHandler.result == -2)
                    return;

                fs.appendFileSync("./debug/_log.txt", `H2H: ${e1Wins} - ${draws} - ${e2Wins}\n`);
                fs.appendFileSync("./debug/_log.txt", `White/Black: ${whiteWins} - ${draws} - ${blackWins}\n`);

                // starts a new double
                startDouble();
            });
        });
    }

    for (let t = 0; t < threads; t++){
        startDouble();
    }
})();
