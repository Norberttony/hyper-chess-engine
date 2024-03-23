// handles elements going in and out of the pool.

var draggingElem = document.getElementById("dragging");
var gameElem = document.getElementById("game");

// fetches an element from the pool
function fetchElem(className, f, r){
    let elem = document.getElementById("element-pool");
    if (!elem){
        elem = document.createElement("div");
    }

    elem.id = "";
    setElemLocation(elem, f, r);
    elem.className = className;

    return elem;
}

// sets the location of an element on the board
function setElemLocation(elem, f, r){
    elem.style.transform = `translate(${(isDisplayFlipped ? 7 - f : f) * 100}%, ${(isDisplayFlipped ? r : 7 - r) * 100}%)`;
}

// either creates a completely new move highlight, or fetches an unused element.
function getMoveHighlightFromPool(f, r){
    return fetchElem("moveHighlight", f, r);
}

// either creates a completely new move highlight, or fetches an unused element.
function getLastMoveHighlightFromPool(f, r){
    return fetchElem("lastMoveHighlight", f, r);
}

// either creates a completely new piece, or fetches an unused element.
function getPieceFromPool(f, r, pieceType, pieceColor){
    let piece = fetchElem("piece", f, r);

    piece.onpointerdown = piecePointerdown;

    piece.id = `${f}_${r}`;

    piece.style.backgroundPositionY = colorToBackground[pieceColor];
    piece.classList.add(PieceTypeToFEN[pieceType]);
    
    return piece;
}

// puts an element back into the pool.
function setElemToPool(elem){
    elem.id = "element-pool";
    elem.className = "";
    elem.innerHTML = "";
    elem.onpointerdown = function(){}
    elem.onpointerup = function(){}
}

// puts a class into the pool
function setClassToPool(classSelector, container = gameElem){
    let elems = container.getElementsByClassName(classSelector);
    while (elems.length > 0)
        setElemToPool(elems[0]);
}

// puts all pieces back into the pool.
function setAllPiecesToPool(container){
    setClassToPool("piece", container);
}

// puts all highlights back into pool.
function setAllMoveHighlightsToPool(container){
    setClassToPool("moveHighlight", container);
}

// puts all last move highlights back into pool.
function setAllLastMoveHighlightsToPool(container){
    setClassToPool("lastMoveHighlight", container);
}
