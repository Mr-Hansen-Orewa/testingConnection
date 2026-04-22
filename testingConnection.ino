#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include "secrets.h"  //double quotes rather than angle brackets makes the compiler look in the sketch folder rather than the library folder
//#include <LibPrintf.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

//------------- WiFi setup --------------
//the Wifi status
int status = WL_IDLE_STATUS;

//Your network name and password are hidden in the secrets.h file
const char SSID[] = SECRET_SSID;
const char PASSWORD[] = SECRET_PASS;

//------------- WebServer setup --------------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object to handle connections on /ws
AsyncWebSocket ws("/ws");

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "inputR";
const char* PARAM_INPUT_2 = "inputG";
const char* PARAM_INPUT_3 = "inputB";

//Variables to save values from HTML form
String inputR;
String inputG;
String inputB;

// File paths to save input values permanently
const char* inputRPath = "/inputR.txt";
const char* inputGPath = "/inputG.txt";
const char* inputBPath = "/inputB.txt";
JSONVar values;

String getCurrentInputValues() {
  values["redVal"] = inputR;
  values["greenVal"] = inputG;
  values["blueVal"] = inputB;
  String jsonString = JSON.stringify(values);
  return jsonString;
}

//------------- Screen setup --------------
Adafruit_ST7789 tftScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//------------- Neopixel setup --------------
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

//these values should be overwritten with 0, 255, 255 from the txt files
//so good error check if red light on neopixel
int red = 255;
int green = 0;
int blue = 0;

//------------- LittleFS setup --------------
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS failed to setup");
  } else {
    Serial.println("LittleFS should be going now");
  }
}

//------------- LittleFS reading a file --------------
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

//------------- LittleFS writing a file --------------
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


//------------- Websocket stuff 1 --------------
//runs whenever we get new data from clients from the web socket pg308
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //saves the data the client sent to the sliderValue variable
    String sliderValue = (char*)data;
    //dutyCycle = map(sliderValue.toInt(), 0, 100, 0, 255);
    // Serial.println(dutyCycle);
  }
}

//------------- Websocket stuff 2 --------------
//event listener handles different asyncronous steps from the web socket
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

//------------- Websocket setup --------------
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  //------------- Serial setup --------------
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

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
  Serial.println("pixel should be going with 000 as its rgb values");

  //------------- Web Socket setup --------------
  initWebSocket();

  //------------- Web Server setup --------------
  // Load values saved in LittleFS
  inputR = readFile(LittleFS, inputRPath);
  inputG = readFile(LittleFS, inputGPath);
  inputB = readFile(LittleFS, inputBPath);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");

  //when the page first loads sends a GET request to /values url respond with the
  //current values of inputR, inputG, and inputB something like
  // "inputR": "0",
  // "inputG": "255",
  // "inputB": "255"
  //from the getCurrentInputValues method up areound line 40 or so
  server.on("/values", HTTP_GET, [](AsyncWebServerRequest* request) {
    String json = getCurrentInputValues();
    request->send(200, "application/json", json);
    json = String();
  });
/* 
//web socket version of above note the String(sliderValue).c_str()
//that requests the slidervalue as a string
server.on("/currentValue", HTTP_GET, [](AsyncWebServerRequest *request){
request->send(200, "/text/plain", String(sliderValue).c_str());
});
*/


  //when you click the submit button search what we were sent in the post request
  //if it is one of the PARAM_INPUT_X 123 we are looking for below then
  //save the value to the inputX RGB variable and to the inputXpath RBG file
  server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        // HTTP POST inputR value
        if (p->name() == PARAM_INPUT_1) {
          inputR = p->value().c_str();
          Serial.print("Input 1 set to: ");
          Serial.println(inputR);
          // Write file to save value
          writeFile(LittleFS, inputRPath, inputR.c_str());
        }
        // HTTP POST inputG value
        if (p->name() == PARAM_INPUT_2) {
          inputG = p->value().c_str();
          Serial.print("Input 2 set to: ");
          Serial.println(inputG);
          // Write file to save value
          writeFile(LittleFS, inputGPath, inputG.c_str());
        }
        // HTTP POST inputB value
        if (p->name() == PARAM_INPUT_3) {
          inputB = p->value().c_str();
          Serial.print("Input 3 set to: ");
          Serial.println(inputB);
          // Write file to save value
          writeFile(LittleFS, inputBPath, inputB.c_str());
        }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.begin();
}

void loop() {

  //Get the RGB values for neopixel
  //purely to make it easier to read in this section
  //they could all just use inputR rather than red etc
  red = inputR.toInt();
  green = inputG.toInt();
  blue = inputB.toInt();

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

  //Just for spamming the button reasons
  delay(2000);

  /////websocket stuff
  ws.cleanupClients();
}
