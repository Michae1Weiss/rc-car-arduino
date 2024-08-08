#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

WebSocketsServer webSocket(81);

const char *ssid  = "softAPI";  // You will connect your phone to this Access Point
const char *pw    = "0675213899"; // and this is the password

IPAddress ip(192, 168, 4, 1);
IPAddress netmask(255, 255, 255, 0);
ESP8266WebServer server(80);

void startWebSocket() { // Start a WebSocket server
  webSocket.begin(); // start the websocket server
  webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED: // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: { // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        if ( payload != NULL) {
          //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        }
      }
      break;
    case WStype_TEXT: // if new text data is received,
      if (payload != NULL) {
        // I used an if statement here for the payload because when the payload was NULL, I was getting some weird errors from the ESP8266 and the device was resetting itself.
        Serial.printf("[%u]%s\n", num, payload);
      }
      break;
  }
}

void setup() {
  Serial.begin(9600);
  SPIFFS.begin();

  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pw); // configure ssid and password for softAP
  startWebSocket();
  Serial.printf("Soft AP created: %s", WiFi.softAPIP().toString().c_str());
  // handle index
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();
}


void loop() {
  webSocket.loop();
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/plain", "Hello World!");
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
