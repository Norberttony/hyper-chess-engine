
const { clearScreenDown } = require("readline");
const { Board } = require("./viewer/scripts/game/game");
const fs = require("fs");

const debugDir = "./debug/";

// handles a single game between white and black
class MatchHandler {
    constructor(white, black, index, fen, finishHandler){
        this.white = white;
        this.black = black;

        // game stats
        this.game = new Board();
        this.wproc = white.createProcess(this.messageHandler.bind(this));
        this.bproc = black.createProcess(this.messageHandler.bind(this));
        this.lastCapture = 0;
        this.positions = {};

        // debug files
        this.debug = `FEN: ${fen}\nWhite: ${white.name}\nBlack: ${black.name}\n`;
        this.wdebug = "";
        this.bdebug = "";

        // results
        this.result = -2;
        this.gameOver = false;

        this.game.loadFEN(fen);

        this.last = performance.now();

        this.index = index;

        this.finish = finishHandler;

        this.wproc.start();
        this.bproc.start();

        this.wproc.write(fen);
        this.bproc.write(fen);

        this.wproc.write("b");
        this.bproc.write("w");

        // wait for a finish
        let interval = 0;
        interval = setInterval(() => {
            if (performance.now() - this.last >= 10000){
                console.log("Something happened.");
                this.debug += "Something happened.";
                this.endOfMatch();
            }

            if (!this.gameOver)
                return;

            clearInterval(interval);
        }, 1000);
    }

    messageHandler(engine, data){
        if (this.gameOver)
            return;

        // update last message
        this.last = performance.now();
        
        // add message onto debug
        if (engine == this.white)
            this.wdebug += data;
        else if (engine == this.black)
            this.bdebug += data;

        // interpret message
        const lines = data.split("\n");

        for (const l of lines){
            const cmds = l.trim().split(" ");

            switch(cmds[0]){
                case "makemove":
                    if (engine == this.white){
                        this.bproc.write(`${cmds[1]}\n`);
                    }else if (engine == this.black){
                        this.wproc.write(`${cmds[1]}\n`);
                    }
                    this.debug += `${cmds[1]}\n`;
                    const m = this.game.playLANMove(cmds[1]);

                    if (!m){
                        console.error("Could not find move");
                    }

                    // fifty move rule stuff
                    this.lastCapture++;
                    if (m && m.captures.length > 0){
                        this.lastCapture = 0;
                    }

                    // for three-fold
                    const pos = this.game.getPosition();
                    if (!this.positions[pos])
                        this.positions[pos] = 0;

                    // 50 moves are equal to 100 halfmoves
                    if (++this.positions[pos] == 3 || this.lastCapture >= 100){
                        this.result = 0;
                        
                        if (this.positions[pos] == 3)
                            this.debug += "threefold\n";
                        else
                            this.debug += "fifty move rule\n";

                        this.wproc.stop();
                        this.bproc.stop();

                        this.endOfMatch();
                    }

                    break;

                case "result":
                    const newResult = resultToNumber(cmds[1]);
                    if (this.result == -2){
                        this.result = newResult;
                    }else if (this.result == newResult){
                        this.debug += "Checkmate/stalemate\n";
                        this.endOfMatch();
                    }else{
                        console.log("ERROR! Bots could not agree on result");
                        this.debug += "Could not agree\n";
                        this.endOfMatch();
                    }
                    break;
            }
        }
    }

    endOfMatch(){
        this.wproc.stop();
        this.bproc.stop();

        this.gameOver = true;

        this.debug += `${this.result}\n`;

        // write debug info
        fs.writeFileSync(`${debugDir}game-${this.index}.txt`, this.debug);

        fs.writeFileSync(`${debugDir}game-${this.index}-${this.white.name}.txt`, this.wdebug);
        fs.writeFileSync(`${debugDir}game-${this.index}-${this.black.name}.txt`, this.bdebug);

        // calls finish handler
        this.finish(this);
    }
}

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

module.exports = { MatchHandler };
