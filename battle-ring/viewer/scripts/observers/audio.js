// Plays some really nice audio.

var containerElem = document.getElementById("container");

const AUDIO = {
    move:           "sounds/move.mp3",
    capture:        "sounds/capture.mp3"
};

// this is a certified goofy solution.
// audio is "buffered" because of a delay when the request for the audio is issued. This acts as a
// cache. Even though the browser automatically caches audio...? eh.
let bufferedAudio = {
    [AUDIO.move]: new Audio(AUDIO.move),
    [AUDIO.capture]: new Audio(AUDIO.capture)
};
function playAudio(src){
    if (bufferedAudio[src]){
        bufferedAudio[src].load();
        bufferedAudio[src].play();
    }else{
        (new Audio(src)).play();
    }
    bufferedAudio[src] = new Audio(src);
    bufferedAudio[src].load();
}

function makeNoise(move){
    if (move.captures.length > 0)
        playAudio(AUDIO.capture);
    else
        playAudio(AUDIO.move);
}

let prevIndex = -1;
containerElem.addEventListener("movescroll", (event) => {
    let {state, board, san, moveIndex} = event.detail;
    
    // only play audio if move scrolling is going forward by exactly 1
    if (moveIndex - prevIndex == 1){
        makeNoise(state.moves[moveIndex]);
    }
    prevIndex = moveIndex;
});
