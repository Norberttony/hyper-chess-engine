// This file handles the way that the user might interact with the game board via dragging pieces.

// make pieces draggable
let currentMoves;
let selected;
let dragging;

// sets the elements dragging position based on the user's pointer position
function setDraggingElemPos(pageX, pageY){
    if (!dragging) return;
    draggingElem.style.left = `calc(${pageX}px - var(--piece-width) / 2)`;
    draggingElem.style.top = `calc(${pageY}px - var(--piece-height) / 2)`;
}

document.addEventListener("pointermove", (event) => {
    if (dragging) event.preventDefault();
    setDraggingElemPos(event.pageX, event.pageY);
});

// prevent scrolling and glitches on mobile devices when thing is being dragged
document.addEventListener("touchmove", (event) => {
    if (dragging) event.preventDefault();
}, {passive: false});

function piecePointerdown(event){
    if (event.button !== undefined && event.button != 0) return;

    setAllMoveHighlightsToPool();

    let coords = this.id.split("_");
    let square = parseInt(coords[0]) + parseInt(coords[1] * 8);

    // if the given piece is currently promoting, the user should not be able to drag it...
    if (this.classList.contains("isPromoting")){
        return;
    }

    // check with gameState if the piece can move
    if (!gameState.canMove(square)) return;

    const piece = gameState.board.squares[square];

    dragging = this;
    selected = this;
    this.classList.add("dragged");
    // copy over graphics
    draggingElem.style.backgroundPositionY = dragging.style.backgroundPositionY;
    // to-do: referencing specific index in classList is unreliable.
    draggingElem.className = "";
    draggingElem.classList.add(dragging.classList[1]);
    draggingElem.style.display = "block";

    setDraggingElemPos(event.pageX, event.pageY);

    // get moves for selected piece and display them
    currentMoves = gameState.board.generatePieceMoves(square, piece);
    for (let i = 0; i < currentMoves.length; i++){
        let move = currentMoves[i];
        
        let highlight = getMoveHighlightFromPool(move.to % 8, Math.floor(move.to / 8));
        highlight.id = `moveHighlight_${i}`;

        // if move is a capture, update highlight graphically to indicate that
        if (move.captures.length > 0)
            highlight.classList.add("capture");

        gameElem.appendChild(highlight);
    }
}

function draggingPointerup(event){
    if (event.button == 2)
        return;

    if (dragging){
        dragging.classList.remove("dragged");
        draggingElem.style.display = "none";
    }
    
    let highlight = event.target;
    
    // handle touch events
    if (event.pointerType == "touch"){
        highlight = document.elementFromPoint(event.clientX, event.clientY);
    }

    // player let go at a highlight, indicating they're moving the piece there.
    if (highlight.classList.contains("moveHighlight")){
        testMove = currentMoves[parseInt(highlight.id.replace("moveHighlight_", ""))];

        // testMove handlers
        if (testMove.promotion)
            handlePromotion(testMove);
        else{
            gameState.makeMove(testMove);
            testMove = undefined;
        }

        // clear all moves from board
        setAllMoveHighlightsToPool();
    }

    if (!dragging){
        // clear all moves from board
        setAllMoveHighlightsToPool();
    }else{
        dragging = undefined;
    }
}

document.addEventListener("pointerup", draggingPointerup);
