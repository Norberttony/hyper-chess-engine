// create a web server so people can join and watch the games!
const express = require("express");
const http = require("http");

const app = express();
const server = http.createServer(app);

const io = require("socket.io")(server);

app.use(express.static(__dirname + "/viewer"));

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

function playGame(white, black, index, fen){
    return new Promise((res, rej) => {

        // create a new match between white and black
        new MatchHandler(white, black, index, fen, (handler) => {
            handler.debug += handler.result;

            // write game file
            fs.writeFileSync(`${debugDir}game-${handler.index}.txt`, handler.debug);
            
            // write debug files
            fs.writeFileSync(`${debugDir}game-${handler.index}-${white.name}.txt`, handler.wdebug);
            fs.writeFileSync(`${debugDir}game-${handler.index}-${black.name}.txt`, handler.bdebug);

            handler.gameOver = true;

            // propagate result
            if (handler.result == -2)
                rej(handler.result);
            else
                res(handler.result);
        });
    });
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
    
    for (let i = (e1Wins + draws + e2Wins) / 2; i < 1000 / 2; i++){
        if (positions.length == 0){
            console.log("out of positions!");
            break;
        }

        console.log("Starting double");
        const chosen = positions[Math.floor(Math.random() * positions.length)];
        console.log("Chose position", chosen);

        // remove position from samples
        positions.splice(positions.indexOf(chosen), 1);

        const res1 = await playGame(engines[0], engines[1], i * 2, chosen.trim());
        const res2 = await playGame(engines[1], engines[0], i * 2 + 1, chosen.trim());

        if (res1 == 1){
            e1Wins++;
            whiteWins++;
        }else if (res1 == -1){
            e2Wins++;
            blackWins++;
        }else{
            draws++;
        }

        if (res2 == 1){
            e2Wins++;
            whiteWins++;
        }else if (res2 == -1){
            e1Wins++;
            blackWins++;
        }else{
            draws++;
        }

        fs.appendFileSync("./debug/_log.txt", `H2H: ${e1Wins} - ${draws} - ${e2Wins}\n`);
        fs.appendFileSync("./debug/_log.txt", `White/Black: ${whiteWins} - ${draws} - ${blackWins}\n`);
    }
})();
