var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", onload);
function onload(event) {
    initWebSocket();
}
function initWebSocket() {
    console.log("Trying to open a WebSocket connection…");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log("WebSocket opened");
}
function onClose(event) {
    console.log("WebSocket closed");
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    var myObj = JSON.parse(event.data);
    console.log(myObj);
    //should be like
    // 0: {pinNum: '2', state: '0'}
    // 1: {pinNum: '13', state: '0'}
    // 2: {r: '0', g: '255', b: '0'}

    var btnPressed = myObj.values[0].state;
    var ledOn = myObj.values[1].state;
    var redVal = myObj.values[2].r;
    var greenVal = myObj.values[2].g;
    var blueVal = myObj.values[2].b;

    //fault checking values read correctly
    console.log(btnPressed);
    console.log(ledOn);
    console.log(redVal);
    console.log(greenVal);
    console.log(blueVal);

    //if d2 button is being pressed change the text on the HTML page
    if (btnPressed == "1") {
        document.getElementById("d2Button").innerHTML = "aren't";
    } else {
        document.getElementById("d2Button").innerHTML = "are";
    }

    //neopixel RGB values pasted over the previous HTML ones
    document.getElementById("redVal").innerHTML = redVal;
    document.getElementById("testColor1").value = redVal;
    document.getElementById("greenVal").innerHTML = greenVal;
    document.getElementById("testColor2").value = greenVal;
    document.getElementById("blueVal").innerHTML = blueVal;
    document.getElementById("testColor3").value = blueVal;

    console.log(event.data);
}

function updatetestColor1(element) {
    var input1 = document.getElementById("testColor1").value;
    document.getElementById("redVal").innerHTML = input1;
    console.log(input1);
    //  websocket.send(input1); //WHAT VALUE AM I SENDING BACK TO THE handleWebSocketMessage METHOD?
    sendRGB();
}

function updatetestColor2(element) {
    var input2 = document.getElementById("testColor2").value;
    document.getElementById("greenVal").innerHTML = input2;
    console.log(input2);
    //  websocket.send(input2);
    sendRGB();
}

function updatetestColor3(element) {
    var input3 = document.getElementById("testColor3").value;
    document.getElementById("blueVal").innerHTML = input3;
    console.log(input3);
    // websocket.send(input3);
    sendRGB();
}

function sendRGB() {
    var input1 = document.getElementById("testColor1").value;
    var input2 = document.getElementById("testColor2").value;
    var input3 = document.getElementById("testColor3").value;

    var rgb = "rgb(" + input1 + "," + input2 + "," + input3 + ")";
    websocket.send(rgb);
}

// Function to get and update GPIO states on the webpage when it loads for the first time
function getStates() {
    //this triggers a notifyClients method in the c++ file which comes back by the onMessage function
    websocket.send("getStates");
}
