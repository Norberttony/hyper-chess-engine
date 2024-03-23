// handles the graphical side of promotion

var promoting = document.getElementsByClassName("promoting")[0];

// this handles prompting the user after test moves that may or may not be promotion
function handlePromotion(move){
    if (!move.promotion) return;

    // let's graphically move the piece and show promotion
    move.promotion = Piece.pawn; // to-do: yeah this is goofy.
    makeMoveGraphically(move);
    move.promotion = "-";
    
    showPromotion(move.to % 8);
}

function promotionClick(){
    // promote piece
    testMove.promotion = parseInt(this.id.replace("promote", ""));

    // to-do: fix DRY violation; repeating with input.js
    gameState.makeMove(testMove);
    testMove = undefined;

    hidePromotion();
}

function showPromotion(file){
    gameElem.style.filter = "blur(2px)";

    // to-do: relying on gamestate allowing is not great.
    let flipPromoting = (isDisplayFlipped && curBoardDisplay.turn == Piece.white) || (!isDisplayFlipped && curBoardDisplay.turn == Piece.black);
    promoting.style.transform = `translate(${(isDisplayFlipped ? 7 - file : file) * 100}%, ${flipPromoting ? "100%" : "0%"})`;
    promoting.style.display = "flex";

    if (flipPromoting)
        promoting.classList.add("flipped");
    else
        promoting.classList.remove("flipped");

    if (curBoardDisplay.turn == Piece.white)
        promoting.classList.remove("black");
    else
        promoting.classList.add("black");
}

function hidePromotion(){
    gameElem.style.filter = "";
    promoting.style.display = "none";
}
