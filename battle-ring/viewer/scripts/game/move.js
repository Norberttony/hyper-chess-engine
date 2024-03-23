// The move class contains all relevant information about any move...
//  - "to"          is a square index of where the moving piece is going to
//  - "from"        is a square index of where the moving piece is moving from
//  - "captures"    is an array of {square, captured} where "square" is the location of the
//                      captured piece and "captured" is the piece itself.

if (typeof(exports) !== "undefined")
    var { squareToAlgebraic } = require("./coords");

class Move {
    constructor(to, from, captures = []){
        this.to = to;
        this.from = from;
        this.captures = captures;
    }
    clone(){
        return new Move(this.to, this.from, [...this.captures]);
    }
    get uci(){
        return `${squareToAlgebraic(this.from)}${squareToAlgebraic(this.to)}`;
    }
}

if (typeof(exports) !== "undefined")
    module.exports = { Move };
