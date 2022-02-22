#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "WsMessage.h"
#include <stdlib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Defines
#define LEDPIN 16
#define BUZZERPIN 12 // pullup or it will crash because of constant interrupting

// Initialize network parameters
const char* SSID = "SSID";
const char* PASSWORD = "PASSWORD";

// NTP
WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

// Initialize server parameters
const char* SERVER_IP = "IP_ADDRESS"; //Replace with IP address
const int SERVER_PORT = 8080; //Replace with server port
const char* PATH = "/"; //Replace with URL path

// Web socket
WebSocketsClient webSocket;

// Bools
bool socketReady = false;
bool registered = false;
bool buzzerPushed = false;
bool firstAttach = true;
bool firstRegister = true;
bool interruptAttached = false;

// Integer
uint32_t timestamp = 0;

// Fetch time from server
void getTimestamp(){
  timestamp = timeClient.getEpochTime();
}

void IRAM_ATTR ISR(){
  getTimestamp();
  buzzerPushed = true;
  switchInterruptAttachment(false);
}

void switchInterruptAttachment(bool attach){
  if(attach){
    detachInterrupt(digitalPinToInterrupt(BUZZERPIN));
  }
  else {
    attachInterrupt(BUZZERPIN, ISR, HIGH);
  }
  interruptAttached = attach;
}

void setup() {
  // Serial monitor
  Serial.begin(115200);

  // Pinmodes
    // interrupt
    pinMode(BUZZERPIN, INPUT);
    
    // LED
    pinMode(LEDPIN, OUTPUT);
    
  // Connecting to WiFi
  uint8_t timeoutCounter = 0;
  WiFi.begin(SSID,PASSWORD);
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED && timeoutCounter < 15) {
    Serial.print(".");
    delay(1000);
    timeoutCounter++;
  }
  Serial.println();
  Serial.print("IP Address: "); 
  Serial.println(WiFi.localIP());

  // NTP client
  timeClient.begin();

  // server address, port, and URL path
  webSocket.begin(SERVER_IP, SERVER_PORT, PATH);

  // websocket event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  Serial.print("Waiting for socket");
}

void loop() {
  // For interrupts
  delay(1); 

  // Update NTP connection
  timeClient.update(); 

  // Initialize after connected to websocket
  if(socketReady && firstRegister){
        registerBuzzer(); 
        firstRegister = false;
  }

  // Attach interrupt when registered
  if(firstAttach && registered){
    switchInterruptAttachment(true);
    firstAttach = false;
  }

  // Send timestamp when buzzer is pressed
  if(buzzerPushed && registered){
     sendTimestamp();
    }
  // Loop websocket connection
  webSocket.loop();
}

void sendTimestamp(){
  // Parse timestamp from uint32 to char* and copy it to buffer
  uint8_t tsLen = 10;
  char* buffer = (char*)calloc(tsLen,sizeof(char));
  utoa(timestamp, buffer, tsLen);
    
  // Send buzzer-pressed packet with timestamp
  sendPacket(WsMessage::EPacketTypes::BUZZER_PRESSED, buffer, tsLen);
  Serial.println("Sent timestamp");
  free(buffer);
  // Reset buzzerPushed
  buzzerPushed = false;
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  // Get and print raw payload
  String payloadString = (const char*)payload;
  Serial.print("payloadString= ");
  Serial.print(payloadString);
  Serial.print(" payloadType= ");
  Serial.println(type);

  // Deserialize message
  if (type == WStype_BIN) {
      Serial.println("Deserialze...");
      StaticJsonDocument<WsMessage::JSON_MAX_LENGTH> inputData;
      deserializeJson(inputData, (char*)payload);
      switch((uint8_t)inputData["packetType"]){
        // Buzzer registered
        case WsMessage::EPacketTypes::BUZZER_REGISTERED:
          registered = true; 
          Serial.println("registered");
          break;
        
        // Buzzer unregistered
        case WsMessage::EPacketTypes::BUZZER_UNREGISTERED:
          registered = false;
          // Intentional fallthrough: also disarm

        // Buzzer disarm
        case WsMessage::EPacketTypes::BUZZER_DISARM:
          switchInterruptAttachment(false);
          break;

        // Buzzer arm
        case WsMessage::EPacketTypes::BUZZER_ARM:
          switchInterruptAttachment(true);
          Serial.println("BUZZER_ARMED");
          break;

        // Led on
        case WsMessage::EPacketTypes::LED_ON:
          digitalWrite(LEDPIN, HIGH);
          Serial.println("LED_ON");
          break; 

        // Led off
        case WsMessage::EPacketTypes::LED_OFF:
          digitalWrite(LEDPIN, LOW);
          Serial.println("LED_OFF");
          break;

        // Emulates buzzer pressing
        case WsMessage::EPacketTypes::BUZZER_PRESSED:
          buzzerPushed = true; 
          Serial.println("Buzzer pressed");
          break;

        // Send wrong-type packet to server
        default:
        Serial.println("Wrong packet type"); 
        sendPacket(WsMessage::EPacketTypes::WRONG_PACKETTYPE);      
      } 
  }

  // Connected to websocket
  else if(type == WStype_CONNECTED){
      Serial.println("Socket is ready");
      socketReady = true;
      Serial.println("Send Client ready");
      webSocket.sendTXT("Client ready");
  
  // Connection lost
  } else if(type == WStype_DISCONNECTED){
      Serial.println("Disconnected. Waiting for reconnect...");
      socketReady = false;
  }
}

// Send register request to server
void registerBuzzer(){
  sendPacket(WsMessage::EPacketTypes::REGISTER_BUZZER);
}

// Send message to server
void sendPacket(WsMessage::EPacketTypes messageType){
  WsMessage* wsMsg = new WsMessage(messageType);
  webSocket.sendTXT(wsMsg->JSON());
}

void sendPacket(WsMessage::EPacketTypes messageType, char* message, uint8_t messageLength){
  WsMessage* wsMsg = new WsMessage(messageType,message,messageLength);
  char* jsonStr = wsMsg->JSON();
  webSocket.sendTXT(jsonStr);
}
