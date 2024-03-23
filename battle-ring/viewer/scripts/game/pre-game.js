// Contains any precomputed logic related to game logic (calculating moves, etc.)

// Precomputes distance to edge of board from every square based on direction.
// organized as N, E, S, W, then NE, SE, SW, NW
var dirOffsets = [8, 1, -8, -1, 9, -7, -9, 7];

var numSquaresToEdge = [];
for (let r = 0; r < 8; r++){
    for (let f = 0; f < 8; f++){
        let n = 7 - r;
        let e = 7 - f;
        let s = r;
        let w = f;

        numSquaresToEdge.push([
            n,
            e,
            s,
            w,
            Math.min(n, e),
            Math.min(s, e),
            Math.min(s, w),
            Math.min(n, w)
        ]);
    }
}

if (typeof(exports) !== "undefined")
    module.exports = { numSquaresToEdge, dirOffsets };
