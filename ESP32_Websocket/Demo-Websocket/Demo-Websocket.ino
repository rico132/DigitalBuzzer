#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>


// Initialize network parameters
const char* SSID = "YOUR_SSID";
const char* PASSWORD = "YOUR_PASSWORD";

// Initialize server parameters
const char* SERVER_IP = "YOUR_SERVER_IP"
const int SERVER_PORT = 81 //Replace with server port
const char* PATH = "/" //Replace with URL path

// Declare websocket client class variable
WebSocketsClient webSocket;

// Allocate the JSON document
StaticJsonDocument<200> doc;

void setup() {
  WiFi.begin(SSID,PASSWORD);
  Serial.begin(115200);
  Serial.print("Connecting");
  uint8_t timeoutCoutner = 0;
  while(WiFi.status() != WL_CONNECTED && timeoutCounter < 15) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  // server address, port, and URL path
  webSocket.begin(SERVER_IP, SERVER_PORT, PATH);

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

}

void loop() {
    webSocket.loop();

}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    // Make sure the screen is clear
    u8g2.clearBuffer();
    if (type == WStype_TEXT)
    {
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds.
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }
        const float t = doc["temperature"];
        const float h = doc["humidity"];
        const float p = doc["pressure"];
        const float g = doc["gas"];

        // Display the Data
        u8g2.drawStr(0, 10, "T:");
        u8g2.drawStr(12, 10, String(t).c_str());
        u8g2.drawStr(0, 22, "H:");
        u8g2.drawStr(12, 22, String(h).c_str());
        u8g2.drawStr(0, 34, "P:");
        u8g2.drawStr(12, 34, String(p).c_str());
        u8g2.drawStr(0, 46, "G:");
        u8g2.drawStr(12, 46, String(g).c_str());

        // Send acknowledgement to the client
        webSocket.sendTXT("{\"status\":\"OK\"}");
    }
    u8g2.sendBuffer();
}
