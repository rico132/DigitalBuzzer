#include "WsMessage.h"
#include <Arduino.h>

WsMessage::WsMessage(EPacketTypes type){
    this->packetType = type;
}

WsMessage::WsMessage(EPacketTypes type, char* message,uint8_t length){
    this->packetType = type;
    this->message = (char*)calloc(length + 1, sizeof(char));
    strncpy(this->message,message,length);
}

WsMessage::~WsMessage(){
    if(this->message)
        free(this->message);
    if(this->outputStr)
        free(this->outputStr);
}   

char* WsMessage::JSON(){
    this->output["packetType"] = this->packetType;
    if(this->message){
        this->output["payload"] = this->message;
        Serial.printf("message: %s\n", this->message);
    }
    else{
        this->output["payload"] = "";
    }
    char* buffer = (char*)calloc(WsMessage::JSON_MAX_LENGTH + 1, sizeof(char));
    serializeJson(this->output,buffer, WsMessage::JSON_MAX_LENGTH + 1);
    uint8_t bufferLen = strnlen(buffer,WsMessage::JSON_MAX_LENGTH);
    this->outputStr = (char*)calloc(bufferLen + 1, sizeof(char));
    strncpy(this->outputStr, buffer, bufferLen);
    free(buffer);
    return this->outputStr;
}