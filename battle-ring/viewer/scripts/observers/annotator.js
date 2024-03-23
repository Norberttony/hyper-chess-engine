// allows user to highlight squares and draw arrows on the board

var containerElem = document.getElementById("container");
var game_containerElem = document.getElementById("game_container");
var gameElem = document.getElementById("game");
var annotationsElem = game_containerElem.getElementsByClassName("annotations")[0];
var annotationsCtx = annotationsElem.getContext("2d");

// for now, stores the currently-on-screen annotations.
let drawnAnnotations = [];

annotationsCtx.lineWidth = 12;
annotationsCtx.strokeStyle = "rgba(0, 120, 0)";
annotationsCtx.fillStyle = "rgba(0, 120, 0)";
annotationsCtx.lineCap = "round";

let annotationStartX;
let annotationStartY;
gameElem.addEventListener("mousedown", (event) => {
    if (event.button == 2){
        annotationStartX = Math.floor(event.layerX / annotationsElem.clientWidth * 8);
        annotationStartY = Math.floor(event.layerY / annotationsElem.clientHeight * 8);

        // if board is flipped, flip the coords too
        if (containerElem.classList.contains("flipped")){
            annotationStartX = 7 - annotationStartX;
            annotationStartY = 7 - annotationStartY;
        }
    }
});

// necessary, unfortunately. clears everything and draws it again.
function redrawUserAnnotations(){
    annotationsCtx.clearRect(0, 0, annotationsElem.width, annotationsElem.height);
    for (let i = 0; i < drawnAnnotations.length; i++){
        let c = drawnAnnotations[i];
        drawAnnotation(parseInt(c[0]), parseInt(c[1]), parseInt(c[2]), parseInt(c[3]));
    }
}

function clearUserAnnotations(){
    drawnAnnotations = [];
    redrawUserAnnotations();
}

function drawAnnotation(startX, startY, endX, endY){
    // general variables useful for drawing annotations
    const squareSize = annotationsElem.width / 8;
    const halfSquare = annotationsElem.width / 16;

    // start and end coordinates in pixel coordinates
    const sx = startX * squareSize + halfSquare;
    const sy = startY * squareSize + halfSquare;
    const ex = endX   * squareSize + halfSquare;
    const ey = endY   * squareSize + halfSquare;

    // representing as a vector
    const x = endX - startX;
    const y = endY - startY;
    const mag = Math.sqrt(x**2 + y**2);
    const nx = x / mag;
    const ny = y / mag;

    // rotated by 90 degrees
    const rx = -ny;
    const ry = nx;

    if (sx == ex && sy == ey){
        // draw just a highlight on the square
        annotationsCtx.lineWidth = 12;
        annotationsCtx.beginPath();
        annotationsCtx.arc(sx, sy, halfSquare - annotationsCtx.lineWidth/2, 0, 2 * Math.PI);
        annotationsCtx.stroke();
    }else{

        const midOffsetX = -50 * nx;
        const midOffsetY = -50 * ny;
        const arrowOffsetX = -80 * nx;
        const arrowOffsetY = -80 * ny;

        // otherwise let's draw an arrow from start to end
        annotationsCtx.lineWidth = 25;
        annotationsCtx.beginPath();
        annotationsCtx.moveTo(sx, sy);
        annotationsCtx.lineTo(ex + midOffsetX, ey + midOffsetY);
        annotationsCtx.stroke();

        annotationsCtx.lineWidth = 0;
        annotationsCtx.beginPath();
        annotationsCtx.moveTo(ex, ey);

        annotationsCtx.lineTo(ex + 50 * rx + arrowOffsetX, ey + 50 * ry + arrowOffsetY);
        annotationsCtx.lineTo(ex - 50 * rx + arrowOffsetX, ey - 50 * ry + arrowOffsetY);

        annotationsCtx.lineTo(ex, ey);
        annotationsCtx.fill();
    }
}

gameElem.addEventListener("mouseup", (event) => {
    if (event.button != 2) return clearUserAnnotations();

    let annotationEndX = Math.floor(event.layerX / annotationsElem.clientWidth * 8);
    let annotationEndY = Math.floor(event.layerY / annotationsElem.clientHeight * 8);

    // flip coords if board flipped too
    if (containerElem.classList.contains("flipped")){
        annotationEndX = 7 - annotationEndX;
        annotationEndY = 7 - annotationEndY;
    }

    let code = `${annotationStartX}${annotationStartY}${annotationEndX}${annotationEndY}`;
    let index = drawnAnnotations.indexOf(code);
    if (index > -1){
        drawnAnnotations.splice(index, 1);
        redrawUserAnnotations();
    }else{
        drawAnnotation(annotationStartX, annotationStartY, annotationEndX, annotationEndY);
        drawnAnnotations.push(code);
    }

    event.preventDefault();
});

gameElem.addEventListener("contextmenu", (event) => {
    event.preventDefault();
});

// observe custom events
containerElem.addEventListener("movescroll", clearUserAnnotations);
