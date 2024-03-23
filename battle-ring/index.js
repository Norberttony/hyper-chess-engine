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

const { Board } = require("./viewer/scripts/game/game");
const { Engine } = require("./engine");

const botsDir = "./bots/";
const debugDir = "./debug/";

let positions = fs.readFileSync("./positions.txt").toString().split("\n");

// remove duplicates
let duplicatesRemoved = 0;
for (let i = 0; i < positions.length; i++){
    let index = positions.indexOf(positions[i], i + 1);
    while (index > -1){
        positions.splice(index, 1);
        duplicatesRemoved++;
        index = positions.indexOf(positions[i], index);
    }
}
console.log(`Removed ${duplicatesRemoved} duplicates`);
console.log(`Left with ${positions.length} positions`);

function resultToNumber(res){
    res = res.trim();
    switch(res){
        case "1-0":
            return 1;
        case "1/2-1/2":
            return 0;
        case "0-1":
            return -1;

        default:
            console.log("UNRECOGNIZED RESULT", res);
    }
}

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
    const game = new Board();
    const positions = {};
    let lastCapture = 0;

    // let clients know
    for (const socket of sockets){
        socket.emit("newgame", fen);
    }

    game.loadFEN(fen);

    let wproc, bproc;
    let wdebug = "";
    let bdebug = "";
    let gameFile = `FEN: ${fen}\nWhite: ${white.name}\nBlack: ${black.name}\n`;

    let result = -2;
    let gameOver = false;

    let last = performance.now();

    finish = () => {
        gameFile += result;

        // write game file
        fs.writeFileSync(`${debugDir}game-${index}.txt`, gameFile);
        
        // write debug files
        fs.writeFileSync(`${debugDir}game-${index}-${white.name}.txt`, wdebug);
        fs.writeFileSync(`${debugDir}game-${index}-${black.name}.txt`, bdebug);

        gameOver = true;
    }

    handler = (engine, data) => {
        if (gameOver)
            return;

        last = performance.now();
        
        if (engine == white)
            wdebug += data;
        else if (engine == black)
            bdebug += data;

        const lines = data.split("\n");

        for (const l of lines){
            const cmds = l.trim().split(" ");

            switch(cmds[0]){
                case "makemove":
                    if (engine == white){
                        bproc.write(`${cmds[1]}\n`);
                    }else if (engine == black){
                        wproc.write(`${cmds[1]}\n`);
                    }
                    gameFile += `${cmds[1]}\n`;
                    const m = game.playLANMove(cmds[1]);

                    console.log(cmds[1]);
                    if (!m){
                        console.error("Could not find move");
                    }else{
                        for (const socket of sockets){
                            socket.emit("move", cmds[1]);
                        }
                    }

                    // fifty move rule stuff
                    lastCapture++;
                    if (m && m.captures.length > 0){
                        lastCapture = 0;
                    }

                    // for three-fold
                    const pos = game.getPosition();
                    if (!positions[pos])
                        positions[pos] = 0;

                    // 50 moves are equal to 100 halfmoves
                    if (++positions[pos] == 3 || lastCapture >= 100){
                        result = 0;
                        
                        if (positions[pos] == 3)
                            console.log("threefold");
                        else
                            console.log("fifty move rule");

                        wproc.stop();
                        bproc.stop();

                        finish();
                    }

                    break;

                case "result":
                    const newResult = resultToNumber(cmds[1]);
                    if (result == -2){
                        result = newResult;
                    }else if (result == newResult){
                        finish();
                    }else{
                        console.log("ERROR! Bots could not agree on result");
                    }
                    break;
            }
        }
    }

    wproc = white.createProcess(handler);
    bproc = black.createProcess(handler);

    wproc.start();
    bproc.start();

    wproc.write(fen);
    bproc.write(fen);

    wproc.write("b");
    bproc.write("w");

    return new Promise((res, rej) => {
        let interval = 0;
        interval = setInterval(() => {
            if (performance.now() - last >= 10000){
                console.log("Something happened.");
                finish();
            }

            if (!gameOver)
                return;

            clearInterval(interval);

            // log result
            console.log(result);

            if (result == -2)
                rej(result);
            else
                res(result);
        }, 1000);
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