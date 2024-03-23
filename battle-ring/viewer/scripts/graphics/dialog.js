var preGameControlsElem = document.getElementById("pre-game-controls");

var dialog_box_containerElem = document.getElementById("dialog-box-container");
var dialog_boxElem = document.getElementById("dialog-box");
var dialog_box_titleElem = document.getElementById("dialog-box-title");
var dialog_box_descElem = document.getElementById("dialog-box-desc");

var result_boxElem = document.getElementById("result-box");
var panel_rematchElem = document.getElementById("panel_rematch");

var invite_popup_containerElem = document.getElementById("invite-popup-container");
var invite_popupElem = document.getElementById("invite-popup");

function showDialogContainer(){
    dialog_box_containerElem.style.display = "flex";
}

function hideDialogContainer(){
    dialog_box_containerElem.style.display = "none";
}

function displayDialogBox(title, desc){
    hideResultBox();

    showDialogContainer();
    dialog_boxElem.style.display = "block";
}

function hideDialogBox(){
    hideDialogContainer();

    dialog_boxElem.style.display = "none";
}

function displayResultBox(result, mewin, termination){
    // change text
    document.getElementById("result-box_result").innerText = result;
    document.getElementById("result_mewin").innerText = mewin;
    document.getElementById("how_span").innerText = termination;

    // hide other boxes
    hideDialogBox();

    // display
    showDialogContainer();
    result_boxElem.style.display = "block";

    // allow player to offer rematch even after closing result box
    panel_rematchElem.style.display = "block";
}

function hideResultBox(){
    result_boxElem.style.display = "none";
    hideDialogContainer();
}

function hideInvite(){
    invite_popup_containerElem.style.display = "none";
    invite_popupElem.style.display = "none";
}
