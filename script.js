/* None of this needed as disabled the buttons in the html
 
//convert a hex value to its rgb equivalent
function hexToRgb(hex) {
    // Parse hex parts into integers
    const r = parseInt(hex.substring(1, 3), 16);
    const g = parseInt(hex.substring(3, 5), 16);
    const b = parseInt(hex.substring(5, 7), 16);

    //returns rgb(255, 0, 0) so may want to just sent 255, 0, 0
    return `rgb(${r}, ${g}, ${b})`;
}

//keeps track of the color picked and converts it to RGB from HEX
const value0 = document.getElementById("testColor0");
value0.addEventListener("input", (e) => {
    //where to send this rgb?
    const rgb = hexToRgb(event.target.value);
});
*/

/*-------- RGB sliders --------*/
/*
//keeps track of the RED value on the slider and updates its output tag with that
const value1 = document.querySelector("#redVal");
const input1 = document.querySelector("#testColor1");
value1.textContent = input1.value;
input1.addEventListener("input", (event) => {
    redVal.textContent = event.target.value;
});

//keeps track of the GREEN value on the slider and updates its output tag with that
const value2 = document.querySelector("#greenVal");
const input2 = document.querySelector("#testColor2");
value2.textContent = input2.value;
input2.addEventListener("input", (event) => {
    greenVal.textContent = event.target.value;
});

//keeps track of the BLUE value on the slider and updates its output tag with that
const value3 = document.querySelector("#blueVal");
const input3 = document.querySelector("#testColor3");
value3.textContent = input3.value;
input3.addEventListener("input", (event) => {
    blueVal.textContent = event.target.value;
});

/*-------- Load values when page first opens --------*/
/*
// Get current sensor readings when the page loads
window.addEventListener("load", getValues);
// Function to get current values on the webpage when it loads/refreshes
function getValues() {
    //Read values from RGB sliders
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse(this.responseText);
            // console.log(myObj);
            //document.getElementById("textFieldValue").innerHTML = myObj.textValue;
            document.getElementById("redVal").innerHTML = myObj.redVal;
            document.getElementById("testColor1").value = myObj.redVal;
            document.getElementById("greenVal").innerHTML = myObj.greenVal;
            document.getElementById("testColor2").value = myObj.greenVal;
            document.getElementById("blueVal").innerHTML = myObj.blueVal;
            document.getElementById("testColor3").value = myObj.blueVal;
        }
    };
    xhr.open("GET", "/values", true);
    xhr.send();
}


 */

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", onload);
function onload(event) {
    initWebSocket();
    getCurrentValue();
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
    console.log(event.data);
}
function getCurrentValue() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
             var myObj = JSON.parse(this.responseText);
            // console.log(myObj);
            //document.getElementById("textFieldValue").innerHTML = myObj.textValue;
            document.getElementById("redVal").innerHTML = myObj.redVal;
            document.getElementById("testColor1").value = myObj.redVal;
            document.getElementById("greenVal").innerHTML = myObj.greenVal;
            document.getElementById("testColor2").value = myObj.greenVal;
            document.getElementById("blueVal").innerHTML = myObj.blueVal;
            document.getElementById("testColor3").value = myObj.blueVal;
        }
    };
    xhr.open("GET", "/values", true);
    xhr.send();
}

function updatetestColor1(element) {
    var input1 = document.getElementById("testColor1").value;
    document.getElementById("redVal").innerHTML = input1;
    console.log(input1);
    websocket.send(input1);
}

function updatetestColor2(element) {
    var input2 = document.getElementById("testColor2").value;
    document.getElementById("greenVal").innerHTML = input2;
    console.log(input2);
    websocket.send(input2);
}

function updatetestColor3(element) {
    var input3 = document.getElementById("testColor3").value;
    document.getElementById("blueVal").innerHTML = input3;
    console.log(input3);
    websocket.send(input3);
}