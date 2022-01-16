//==================================
//ESP32 WebSocket Server: Toggle LED
//by: Ulas Dikme
//==================================
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
//-----------------------------------------------
const char* SSID = "SSID"; 
const char* PASSWORD = "PASSWORD";
//-----------------------------------------------
#define LED 2
//-----------------------------------------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//-----------------------------------------------
StaticJsonDocument<200> outputData;
StaticJsonDocument<200> inputData;


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
   


  String payloadString = (const char *)payload;
  Serial.print("payloadString= ");
  Serial.println(payloadString);
  
  const char* str;
  if(type == WStype_TEXT){
    deserializeJson(inputData,(char *)payload);
    str = inputData["message"];
    Serial.print("Received: ");
    Serial.println(str);
    if(strcmp(str,"Hello Server")==0){
        sendPong();
    }
  }

  /*if(type == WStype_TEXT) //receive text from client
  {
    byte separator=payloadString.indexOf('=');
    String var = payloadString.substring(0,separator);
    Serial.print("var= ");
    Serial.println(var);
    String val = payloadString.substring(separator+1);
    Serial.print("val= ");
    Serial.println(val);
    Serial.println(" ");

    if(var == "LEDonoff")
    {
      LEDonoff = false;
      if(val == "ON") LEDonoff = true;
    }
  }*/
}

void sendPong(){
    outputData["message"] = "Hello Client";
    char JSONstr[50];
    serializeJson(outputData,JSONstr);
    webSocket.broadcastTXT(JSONstr);
    Serial.print("Sent: ");
    Serial.println((const char*)outputData["message"]);
    
}
