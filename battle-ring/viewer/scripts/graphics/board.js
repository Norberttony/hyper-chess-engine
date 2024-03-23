// Initializes the graphical board by generating HTML elements
// quite some kolmogorov complexity, though I certainly would not call it code golf.

var gameElem = document.getElementById("game");
var game_containerElem = document.getElementById("game_container");

// displays certain board features (ranks, files, tiles, etc.)
let filesElem = document.getElementById("files");
for (const c of "abcdefgh"){
    let file = document.createElement("div");
    file.innerText = c;
    filesElem.appendChild(file);
}

let ranksElem = document.getElementById("ranks");
for (let r = 1; r <= 8; r++){
    let rank = document.createElement("div");
    rank.innerText = r;
    ranksElem.appendChild(rank);
}

// graphical promotion ui
var promotingElem = document.getElementsByClassName("promoting")[0];
for (let i = 2; i <= 5; i++){
    let p = document.createElement("div");
    p.id = `promote${i}`;
    p.classList.add(PieceTypeToFEN[i]);
    promotingElem.appendChild(p);

    p.addEventListener("click", promotionClick);
}
