#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>


// Initialize network parameters
const char* SSID = "SSID"; 
const char* PASSWORD = "PASSWORD";


// Initialize server parameters
const char* SERVER_IP = "SERVER_IP";
const int SERVER_PORT = 81; //Replace with server port
const char* PATH = "/"; //Replace with URL path

// Declare websocket client class variable
WebSocketsClient webSocket;

// Allocate the JSON document
StaticJsonDocument<200> outputData;
StaticJsonDocument<200> inputData;

uint16_t startMillis;
uint16_t currentMillis;
const uint16_t period = 2000;

void setup() {
  WiFi.begin(SSID,PASSWORD);
  Serial.begin(115200);
  Serial.print("Connecting");
  uint8_t timeoutCounter = 0;
  while(WiFi.status() != WL_CONNECTED && timeoutCounter < 15) {
    Serial.print(".");
    delay(1000);
    timeoutCounter++;
  }
  Serial.println();
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  // server address, port, and URL path
  webSocket.begin(SERVER_IP, SERVER_PORT, PATH);

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  
  startMillis = millis();
  Serial.print(startMillis);
  Serial.print("Waiting for socket");
  
}

void loop() {
  
  while(currentMillis - startMillis <= period){
    webSocket.loop();
    
    currentMillis = millis();
    Serial.print(".");
    Serial.print(currentMillis - startMillis);
    
  }
  /*currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period && currentMillis - startMillis <= period*1.1)  //test whether the period has elapsed
  {*/
    sendPing();
    webSocket.loop();
    
  /*}
  else if(currentMillis - 2 * startMillis >= period){
    Serial.println(".");
  }
  else{
    Serial.print(".");
  }
  */
}
    
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
   // get information from server
    String payloadString = (const char*)payload;
    Serial.print("payloadString= ");
    Serial.println(payloadString);

    const char* str;
    if (type == WStype_TEXT) {
        deserializeJson(inputData, (char*)payload);
        str = inputData["message"];
        Serial.print("Received: ");
        Serial.println(str);
        if (strcmp(str, "Hello Client") == 0) {
            sendPing();
        }
    }
}

void sendPing() {
    outputData["message"] = "Hello Server";
    char JSONstr[50];
    serializeJson(outputData, JSONstr);
    //Serial.println(JSONstr);
    webSocket.sendTXT(JSONstr);
    Serial.print("Sent: ");
    Serial.println((const char*)outputData["message"]);
    delay(1000);
}
