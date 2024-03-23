// handles displaying any of the moves in a separate panel, allowing the user to not only scroll
// through the various moves, but also scroll to a specific move. Most of the "move scrolling"
// logic is performed in a global instance of class GraphicalState.

var containerElem = document.getElementById("container");

let pgnElem = document.getElementById("pgn");

var pgnText = document.getElementById("pgnText");
var fenText = document.getElementById("fenText");

function selectPGNElem(elem){
    (document.getElementsByClassName("pgn_selected")[0] || elem || document.body).classList.remove("pgn_selected");
    if (elem){
        elem.classList.add("pgn_selected");
        
        // scrolls to selected pgn
        let pgnRect = pgnElem.getBoundingClientRect();
        let elemRect = elem.getBoundingClientRect();
        let elemPos = elemRect.top - pgnRect.top; // element's position relative to the PGN scroll view

        // already in view! don't scroll!
        if (elemPos > 0 && elemPos + elemRect.height < pgnRect.height)
            return;

        // determine direction of scroll
        if (elemPos <= 0){
            // so we've scrolled past element, set it to top (as that is first occurrence)
            pgnElem.scrollBy(0, elemRect.top - pgnRect.top);
        }else{
            pgnElem.scrollBy(0, elemRect.bottom - pgnRect.bottom);
        }
    }else{
        pgnElem.scrollTo(0, 0);
    }
}

pgnText.value = gameState.pgnData.toString();
containerElem.addEventListener("pgnMadeMove", (event) => {

    // expects pgnMove to always have a prev
    const {state, board, san, move, pgnMove} = event.detail;

    if (pgnMove.element)
        return;

    let selectedContainer = pgnElem;

    // whether or not the pgn_san element itself needs a counter text added to it
    let includeCounter = false;

    // Handle main variation moves!
    if (pgnMove.isMain()){
        // sometimes, a user loads a position where it is black to play, offsetting all PGN.
        const isOffset = pgnMove.prev == gameState.moveRoot && !gameState.pgnData.startedWTP;

        // adds counter (or number next to every two ply)
        if (board.turn == Piece.black || isOffset){
            let pgn_counterElem = document.createElement("pgn_counter");
            pgn_counterElem.classList.add("pgn_counter");
            pgn_counterElem.innerText = `${board.fullmove}.`;
            pgnElem.appendChild(pgn_counterElem);

            if (isOffset){
                let pgn_elipsesElem = document.createElement("div");
                pgn_elipsesElem.classList.add("pgn_counter");
                pgn_elipsesElem.innerText = "...";
                pgn_elipsesElem.style.textAlign = "left";
                pgnElem.appendChild(pgn_elipsesElem);

                pgn_counterElem.innerText = `${board.fullmove - 1}.`;
            }
        }
    }
    // Handle non-main variation moves!
    else{
        // is this a new variation?
        if (pgnMove.location[pgnMove.location.length - 1] > 0){
            includeCounter = true;

            // yes! yes it is!
            const varElem = document.createElement("div");
            varElem.classList.add("pgn_variation");

            let beforeNode;
            if (pgnMove.prev.next[0].next[0]){
                console.log("select beforeNode by going prev next next");
                beforeNode = pgnMove.prev.next[0].next[0].element;
            }

            // don't cut off the pgn fullmove counter from the SAN
            if (beforeNode && beforeNode.previousSibling){
                const prevNode = beforeNode.previousSibling;
                if (prevNode.classList.contains("pgn_counter")){
                    beforeNode = prevNode;
                    console.log("changed my mind on beforeNode, do not cut off the fullmove counter");
                }
                console.log(beforeNode.previousSibling);
            }

            if (pgnMove.prev.next[0].element.previousSibling.classList.contains("pgn_counter") || beforeNode && beforeNode.previousSibling && beforeNode.previousSibling.previousSibling && beforeNode.previousSibling.previousSibling.classList.contains("pgn_counter")){

                // wait. no fullmove counter?! if this is the main variation, that means we're cutting off a move!
                // make sure to only do this once per variation though
                if (pgnMove.prev.isMain() && pgnMove.location[pgnMove.location.length - 1] == 1){
                    const container = pgnMove.prev.next[0].element.parentNode;

                    const blankMove = document.createElement("div");
                    blankMove.classList.add("pgn_san", "pgn_blank");
                    blankMove.style.pointerEvents = "none";
                    blankMove.innerText = "...";
                    container.insertBefore(blankMove, beforeNode);

                    const counterElem = document.createElement("div");
                    counterElem.classList.add("pgn_counter");
                    counterElem.innerText = `${board.fullmove}.`;
                    container.insertBefore(counterElem, blankMove);

                    beforeNode = counterElem;
                    console.log("set beforeNode to a newly created counter that goes past this variation");
                }
            }

            if (beforeNode){
                console.log("beforeNode", beforeNode);
                beforeNode.parentNode.insertBefore(varElem, beforeNode);
            }else{
                if (pgnMove.prev == gameState.moveRoot){
                    console.log("is root");
                    pgnElem.insertBefore(varElem, pgnElem.lastChild.previousSibling.previousSibling);
                }else{
                    pgnMove.prev.element.parentNode.appendChild(varElem);
                }
            }

            selectedContainer = varElem;
        }
        // not a new variation
        else{
            selectedContainer = pgnMove.prev.element.parentNode;

            // black just played a move
            if (board.turn == Piece.black)
                includeCounter = true;
        }
    }

    let counterText = "";
    if (includeCounter){
        // black just played a move
        if (board.turn == Piece.black){
            counterText = `${board.fullmove}. `;
        }else{
            counterText = `${board.fullmove - 1}... `;
        }
    }

    // now add the move itself
    const pgn_sanElem = document.createElement("div");
    pgn_sanElem.classList.add("pgn_san");
    pgn_sanElem.innerText = `${counterText}${san}`;
    selectedContainer.appendChild(pgn_sanElem);
    pgnMove.element = pgn_sanElem;

    pgn_sanElem.addEventListener("click", () => {
        state.setMove(pgnMove);
    });

    pgnText.value = gameState.pgnData.toString();
});

containerElem.addEventListener("result", (event) => {
    const {state, board} = event.detail;

    // convert symbols of / and # into words
    let result = board.result;
    if (result == "/") result = "1/2 - 1/2";
    else if (board.result == "#" && board.turn == Piece.white) result = "0 - 1";
    else if (board.result == "#" && board.turn == Piece.black) result = "1 - 0";

    // gameState.pgnData.addToSAN(result);

    // get some flavor text based on result
    let flavorText;
    if (result == "0 - 1"){
        flavorText = "Black wins by";
    }else if (result == "1 - 0"){
        flavorText = "White wins by";
    }else if (result == "1/2 - 1/2"){
        flavorText = "Game ended by";
    }

    // displays result
    const pgn_resultElem = document.createElement("div");
    pgn_resultElem.classList.add("pgn_result");
    pgn_resultElem.innerHTML = `<span>${result}</span><br /><span style = "font-size: large;">${flavorText} ${board.termination}</span>`;
    pgnElem.appendChild(pgn_resultElem);

    pgnText.value = gameState.pgnData.toString();
});

containerElem.addEventListener("loadedFEN", (event) => {
    const {state, fen} = event.detail;

    // none of the previous PGN is relevant to this new position, so...
    clearPGNView();
    gameState.pgnData.clear();

    if (fen.replace(/ /g, "") != StartingFEN.replace(/ /g, "")){
        gameState.pgnData.setHeader("Variant", "From Position");
        gameState.pgnData.setHeader("FEN", fen);
    }

    fenText.value = state.board.getFEN();
    pgnText.value = gameState.pgnData.toString();
});

containerElem.addEventListener("movescroll", (event) => {
    const {state, board, pgnMove} = event.detail;

    selectPGNElem(pgnMove.element);

    fenText.value = board.getFEN();
});
fenText.value = StartingFEN;

function clearPGNView(){
    pgnElem.innerHTML = "";
}

let selectedVariation = 0;
function PGNMoveBack(){
    if (gameState.previousMove())
        gameState.graphicsUpdate();
    selectedVariation = 0;
}

function PGNMoveForward(){
    if (gameState.nextMove(selectedVariation))
        gameState.graphicsUpdate();
    selectedVariation = 0;
}

function PGNUpVariation(){
    selectedVariation--;
    if (selectedVariation < 0){
        selectedVariation = gameState.currentMove.next.length - 1;
    }
}

function PGNDownVariation(){
    selectedVariation = (selectedVariation + 1) % gameState.currentMove.next.length;
}

function PGNMoveFirst(){
    // displays position with no moves made on the board
    gameState.setMove(gameState.moveRoot);
}

function PGNMoveLast(){
    // displays position of the last committed move in the main variation
    let iter = gameState.moveRoot;
    while (iter.next[0]){
        iter = iter.next[0];
    }

    gameState.setMove(iter);
}

// key binds for scrolling through moves
document.body.addEventListener("keydown", function(event){
    if (event.target == document.body){
        switch(event.key.toLowerCase()){
            case "arrowleft":
                PGNMoveBack();
                break;
            case "arrowright":
                PGNMoveForward();
                break;
            case "arrowup":
                PGNUpVariation();
                //PGNMoveFirst();
                break;
            case "arrowdown":
                PGNDownVariation();
                //PGNMoveLast();
                break;

            case "f":
                flipBoard();
                break;

            // if no bound key was hit, do not prevent event default.
            default:
                return;
        }
        event.preventDefault();
    }
});
