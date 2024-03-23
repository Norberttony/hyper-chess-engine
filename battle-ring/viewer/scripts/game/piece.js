// Contains all logic for pieces, such as their numerical representations. FEN to piece conversions
// are also stored here.
var Piece = {
    none: 0,
    king: 1,
    retractor: 2,
    chameleon: 3,
    springer: 4,
    coordinator: 5,
    straddler: 6,
    immobilizer: 7,
    white: 8,
    black: 16,

    pieceType: 0b00111,
    colorType: 0b11000,

    getType: function(p){
        return p & Piece.pieceType;
    },
    setType: function(p, t){
        return this.getColor(p) | t;
    },
    ofType: function(p, o){
        return Piece.getType(p) == Piece.getType(o);
    },
    getColor: function(p){
        return p & Piece.colorType;
    },
    ofColor: function(p, o){
        return Piece.getColor(p) == Piece.getColor(o);
    }
};

var PieceASCII = ["?", "K", "Q", "B", "N", "R", "P", "U"];

var FENToPiece = {
    k: Piece.black | Piece.king,
    q: Piece.black | Piece.retractor,
    b: Piece.black | Piece.chameleon,
    n: Piece.black | Piece.springer,
    r: Piece.black | Piece.coordinator,
    p: Piece.black | Piece.straddler,
    u: Piece.black | Piece.immobilizer,

    K: Piece.white | Piece.king,
    Q: Piece.white | Piece.retractor,
    B: Piece.white | Piece.chameleon,
    N: Piece.white | Piece.springer,
    R: Piece.white | Piece.coordinator,
    P: Piece.white | Piece.straddler,
    U: Piece.white | Piece.immobilizer
};

var PieceTypeToFEN = {
    [Piece.king]: "k",
    [Piece.retractor]: "q",
    [Piece.chameleon]: "b",
    [Piece.springer]: "n",
    [Piece.coordinator]: "r",
    [Piece.straddler]: "p",
    [Piece.immobilizer]: "u"
};

// for graphical purposes
var colorToBackground = {
    [Piece.white]: "0%",
    [Piece.black]: "100%"
};

if (typeof(exports) !== "undefined")
    module.exports = { Piece, PieceTypeToFEN, FENToPiece, PieceASCII };
