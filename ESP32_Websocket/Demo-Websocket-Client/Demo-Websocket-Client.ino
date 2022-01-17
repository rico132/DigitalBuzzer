#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

//Define pins
#define BUZZERPIN 12
#define LEDPIN 13

// Initialize network parameters
const char* SSID = "SSID";
const char* PASSWORD = "PASSWORD";


// Initialize server parameters
const char* SERVER_IP = "192.168.178.37";
const int SERVER_PORT = 81; //Replace with server port
const char* PATH = "/"; //Replace with URL path

// Web socket
WebSocketsClient webSocket;
uint8_t socketReady = 0;

// Allocate the JSON document
StaticJsonDocument<200> outputData;
StaticJsonDocument<200> inputData;

//Configure waiting period
uint16_t startMillis = 0;
uint16_t currentMillis = 0;
const uint16_t period = 2000;


//Buzzer 
struct buzzer {
    uint8_t addr = 0;
    char timestamp[50];
    char team[10];
    uint8_t buzzerPushed = 0;
    uint8_t initialized = 0;
};

buzzer buzzer;
/*uint8_t addr = 0;
uint8_t buzzerPushed = 0;
uint8_t initialized = 0;*/

void IRAM_ATTR ISR(){
    buzzer.buzzerPushed = 1;
}

void setup() {
  Serial.begin(115200);

  //interrupt
  pinMode(BUZZERPIN, INPUT);
  attachInterrupt(BUZZERPIN, ISR, HIGH);
  
  //LED
  pinMode(LEDPIN, OUTPUT);
  
  //Connecting to WiFi
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

  // server address, port, and URL path
  webSocket.begin(SERVER_IP, SERVER_PORT, PATH);

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  Serial.print("Waiting for socket");
  
}

void loop() {
  delay(1);

  if(socketReady != 0 && buzzer.initialized == 0){
        Serial.println("Go to init()");
        initialize();
   }

  if(buzzer.buzzerPushed != 0){
      sendPing();
      buzzer.buzzerPushed = 0;
  }
  webSocket.loop();
}
    
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
   // get information from server
    String payloadString = (const char*)payload;
    Serial.print("payloadString= ");
    Serial.print(payloadString);
    Serial.print(" payloadType= ");
    Serial.println(type);

    const char* str;
    if (type == WStype_TEXT) {
        Serial.println("Deserialze...");
        deserializeJson(inputData, (char*)payload);
        //Serial.println((const unsigned char*)inputData["adress"]);
        //char str[10];
        //strncpy(str, (const char*) inputData["adress"], 10);
        buzzer.addr = atoi((const char*)inputData["address"]);
        //Serial.println(str);        
        //buzzer.addr = atoi(str);
        Serial.print("Buzzer addr: ");
        Serial.println(buzzer.addr);
        /*if (buzzer.addr != 0) {
            buzzer.initialized = 1;
            Serial.println("Initialized.");
        }*/
        Serial.print("Received: ");
        Serial.println(buzzer.addr);        
    }
    else if(type == WStype_CONNECTED){
        Serial.println("Socket is ready");
        socketReady = true;
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
    
}

void initialize(){
    StaticJsonDocument<200> init;
    
    char str[10];
    itoa(buzzer.addr,str,10);
    Serial.println(str);
    init["address"] = str;
    Serial.println((const char*)init["adress"]);
    //init["timestamp"] = millis();
    init["team"] = "1";
    
    char JSONstr[50];
    serializeJson(init, JSONstr);
    Serial.print("Sent: ");
    Serial.println(JSONstr);
    webSocket.sendTXT(JSONstr);
    Serial.println("Initializing...");
    buzzer.initialized = 1;
}
