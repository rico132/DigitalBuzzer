
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <vector>

const char* SSID = "SSID"; 
const char* PASSWORD = "PASSWORD";

#define LED 2

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

StaticJsonDocument<200> outputData;
StaticJsonDocument<200> inputData;

//struct buzzer
struct buzzer {
    uint8_t addr = 0;
    char timestamp[50];
    char team[10];
    uint8_t buzzerPushed = 0;
    uint8_t initialized = 0;
};

std::vector<buzzer> buzzerList;

void setup()
{
  Serial.begin(115200); 
  pinMode(LED, OUTPUT);
  //-----------------------------------------------
  WiFi.begin(SSID,PASSWORD);
  uint8_t timeoutCounter = 0;
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED && timeoutCounter < 15) {
    Serial.print(".");
    delay(1000);
    timeoutCounter++;
  }
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  //-----------------------------------------------
  server.on("/", sendPong);
  //-----------------------------------------------
  server.begin(); 
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
//====================================================================
void loop()
{
  webSocket.loop(); 
  server.handleClient();
  //-----------------------------------------------
  /*if(LEDonoff == false) digitalWrite(LED, LOW);
  else digitalWrite(LED, HIGH);*/
  //-----------------------------------------------
  
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t welength)
{
   

    if (payload != nullptr) {
      Serial.print("payloadString= ");
      String payloadString = (const char *)payload;
      Serial.println(payloadString);
    }
  
  const char str[] = "test";
  if(type == WStype_TEXT){
    deserializeJson(inputData,(char *)payload);
    if (strcmp(inputData["address"], "0") == 0) {
        buzzer buzzer;
        buzzer.addr = buzzerList.size() + 1;
        Serial.print("buzzer addr: ");
        Serial.println(buzzer.addr);
        char str[10];
        itoa(buzzerList.size() + 1, str, 10);
        strncpy((char*)buzzer.team, str, 10);
        Serial.print("buzzer team: ");
        Serial.println(buzzer.team);
        buzzer.initialized = 1;
        //buzzer.team = str; 
        buzzerList.push_back(buzzer);
        configureBuzzer(&buzzerList.back());
    }
    if(strcmp(str,"Hello Server")==0){
        sendPong();
    }
  }

}

void sendPong(){
    outputData["message"] = "Hello Client";
    char JSONstr[50];
    serializeJson(outputData,JSONstr);
    webSocket.broadcastTXT(JSONstr);
    Serial.print("Sent: ");
    Serial.println((const char*)outputData["message"]);
    
}

void configureBuzzer(buzzer* buzzer) {
    StaticJsonDocument<200> config;
    char str[10];
    itoa(buzzer->addr, str, 10);
    Serial.print("config buzzer addr: ");
    Serial.println(str);
    config["address"] = str;
    for (int i = 0; i < strlen(str); i++) {
        str[i] = '\0';
    }
    config["team"] = buzzer->team;
    itoa(buzzer->initialized, str, 10);
    Serial.print("config buzzer init: ");
    Serial.println(str);
    config["init"] = str;
    char JSONstr[50];
    serializeJson(config, JSONstr);
    Serial.print("config buzzer JSONstr: ");
    Serial.println(JSONstr);
    webSocket.broadcastTXT(JSONstr);
    Serial.println("leaving configureBuzzer");
}
