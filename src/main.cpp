#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>
#include <index.html.gz.h>

char last_modified[50];

// Neopixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, 12, NEO_GRB + NEO_KHZ800);
// WiFi config
const char *SSID = "Wifi SSID";
const char *PWD = "Password";

// Web server running on port 80
AsyncWebServer server(80);
// Web socket
AsyncWebSocket ws("/ws");


void handleNotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
 
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
 
  request->send(404, "text/plain", message);
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }
 
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}


void handlingIncomingData(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String hexColor = "";
    for (int i=0; i < len; i++)
      hexColor += ((char) data[i]);
    Serial.println(len);

    Serial.println("Hex Color: " + hexColor);
    Serial.println(&hexColor[1]);
    Serial.println(hexColor);
    long n = strtol(&hexColor[1], NULL, 16);
    Serial.println(n);
    strip.fill(n);
    strip.show();
  }
}

// Callback for incoming event
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, 
             void * arg, uint8_t *data, size_t len){
   switch(type) {
      case WS_EVT_CONNECT:
        Serial.printf("Client connected: \n\tClient id:%u\n\tClient IP:%s\n", 
             client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
         Serial.printf("Client disconnected:\n\tClient id:%u\n", client->id());
         break;
      case WS_EVT_DATA:
         handlingIncomingData(arg, data, len);
         break;
      case WS_EVT_PONG:
          Serial.printf("Pong:\n\tClient id:%u\n", client->id());
          break;
      case WS_EVT_ERROR:
          Serial.printf("Error:\n\tClient id:%u\n", client->id());
          break;     
   }
  
}

void handleRoot(AsyncWebServerRequest *request) {
    const char* dataType = "text/html";
 
    Serial.println("Stream the array!");
 
    AsyncWebServerResponse *response = request->beginResponse_P(200, dataType,index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}


void setup() {
  Serial.begin(9600);

  strip.begin();
  strip.setBrightness(255);
  strip.fill(strip.Color(0,0,0));
  strip.show();

  connectToWiFi();
  ws.onEvent(onEvent);
  server.addHandler(&ws);
 
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  ws.cleanupClients();
}
