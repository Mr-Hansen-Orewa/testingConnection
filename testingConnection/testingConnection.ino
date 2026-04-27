#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include "secrets.h"  //double quotes rather than angle brackets makes the compiler look in the sketch folder rather than the library folder
//#include <LibPrintf.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

//------------- D2 button setup --------------
const byte BTNPIN = 2;  //D2 is also GPIO2 //Oh not zero //D0 should be avoided
const byte LEDPIN = 13;

//------------- Screen setup --------------
Adafruit_ST7789 tftScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


//------------- Neopixel setup --------------
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

//these values give a green light on neopixel
int red = 0;
int green = 255;
int blue = 0;


//------------- WiFi setup --------------
//the Wifi status
//int status = WL_IDLE_STATUS;

//Your network name and password are hidden in the secrets.h file
const char SSID[] = SECRET_SSID;
const char PASSWORD[] = SECRET_PASS;

//------------- WebServer setup --------------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");


/**
LittleFS setup - wants to be first method for some reason?
*/
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS failed to setup");
  } else {
    Serial.println("LittleFS should be going now");
  }
}

/**
LittleFS reading a file 
*/
String readFile(fs::FS& fs, const char* path) {
  Serial.print("Reading file: ");
  Serial.println(path);
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("readFile failed to open file");
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

/**
LittleFS writing a file
*/
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.print("Writing file: ");
  Serial.println(path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("writeFile failed to write to file");
    return;
  }
  if (file.print(message)) {
    Serial.println("- writeFile success");
  } else {
    Serial.println("- writeFile failed");
  }
}

/**
set up a JSON array and hold the states of all values we care about
pins and their current states variables and their current values
*/
String getOutputStates() {
  JSONVar myArray;

  myArray["values"][0]["pinNum"] = String(2);
  myArray["values"][0]["state"] = String(digitalRead(2));

  myArray["values"][1]["pinNum"] = String(13);
  myArray["values"][1]["state"] = String(digitalRead(13));

  myArray["values"][2]["r"] = String(red);
  myArray["values"][2]["g"] = String(green);
  myArray["values"][2]["b"] = String(blue);

  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

/**
Sends a message to all clients connected to ip address with a JSON message
*/
void notifyClients(String state) {
  ws.textAll(state);
}

/**
Handles messages from clients in the if statements below
*/
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //if message is states send the state of all gpio pins
    if (strcmp((char*)data, "getStates") == 0) {
      notifyClients(getOutputStates());
    } else {
      //(char*)data should be in this format "rgb(255,87,51)" when it isn't 'getStates'
      std::string rgb = (char*)data;
      byte firstComma = rgb.find(',');
      byte secondComma = rgb.find(',', firstComma + 1);
      byte lastBracket = rgb.find(')');

      red = stoi(rgb.substr(4, firstComma));
      green = stoi(rgb.substr(firstComma + 1, secondComma - firstComma - 1));
      blue = stoi(rgb.substr(secondComma + 1, lastBracket));
    }
    //Just to alternate the led on and off to show we can digitalRead and digitalWrite here
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
}

/**
Event listener handles different asyncronous steps from the web socket
*/
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      Serial.print("WebSocket client ");
      Serial.print(client->id());
      Serial.print(" connected from ");
      Serial.println(client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Serial.print("WebSocket client ");
      Serial.print(client->id());
      Serial.println(" disconnected\n");
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


/**
WebSocket setup
What to do when an event happens on the ws socket and adding a handler to the server
*/
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  Serial.println("websocket should be running now");
}


/**
 Setup serial, pins, wifi, littleFS, websocket, websever, screen, and neopixel
*/
void setup() {
  //------------- Serial setup --------------
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  //------------- Button and LED setup --------------
  pinMode(BTNPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);  //default led not neopixel

  //----------- WiFi setup ------------
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Trying to connect WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(".");
  if (WL_CONNECTED == true) {
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Issues showing the IP address");
  }
  Serial.println("WiFi should be going now");

  //----------- LittleFS setup ------------
  initLittleFS();

  //----------- webSocket setup ------------
  initWebSocket();

  //------------- Web Server setup --------------
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html", false);
  });
  server.serveStatic("/", LittleFS, "/");

  server.begin();
  Serial.println("webserver should be running now");

  //------------- Screen setup --------------
  pinMode(TFT_BACKLITE, OUTPUT);
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  digitalWrite(TFT_I2C_POWER, HIGH);
  tftScreen.init(135, 240);
  tftScreen.setRotation(3);
  tftScreen.fillScreen(ST77XX_WHITE);
  Serial.println("screen should be going with a white background");

  //------------- Neopixel setup --------------
  pixels.begin();
  pixels.show();
  Serial.println("pixel should be going with 000 as its rgb values");  //but when is it updated? and where
}

/**
Loop cleaning clients, and displaying/setting the neopixel color
*/
void loop() {
  //frees up memory by refreshing the list of clients connected
  ws.cleanupClients();
  //delay(1000); //will effect any button click i think

  //seems to spam the webpage with the states so like the serial monitor
  //give it a delay to slow this traffic down?
  //picks up button click real quick though :)
  notifyClients(getOutputStates()); //AM I AN ISSUE NEED TO TEST--------------------------------------


  //------------- Display RGB values --------------
  //Give feedback in serial monitor about RGB values
  Serial.print("RGB value is ");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.println(blue);

  //or you could use printf in the serial monitor NB need commented out library LibPrintf
  //printf("RGB value is %i, %i, %i", red, green, blue);

  //Give feedback on tft screen about RGB values
  tftScreen.fillScreen(ST77XX_WHITE);  //stops text going over itself on next loop
  tftScreen.setCursor(0, 0);
  tftScreen.setTextColor(ST77XX_BLACK);
  tftScreen.setTextSize(2);
  tftScreen.setTextWrap(true);
  tftScreen.println(WiFi.localIP());
  tftScreen.print("RGB value is ");
  tftScreen.print(red);
  tftScreen.print(",");
  tftScreen.print(green);
  tftScreen.print(",");
  tftScreen.println(blue);

  //------------- Setter --------------
  //Set the neopixel to the chosen colour
  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();
}
