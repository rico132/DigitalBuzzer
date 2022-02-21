#include <ArduinoJson.h>
class WsMessage
{
public:
    enum EPacketTypes  {
        RESPONSE_PACKET,
        /**
         * This one is used, when no game is started yet
         */
        REGISTER_MASTER,
        REGISTER_SCREEN,
        PRESETUP_AVAILABLE_INFO,
        SETUP_GAME,
        /**
         * This one is sent from the server, if a master tries to register to an existing game that has no master anymore
         */
        NEW_MASTER_ACCEPTED,
        START_GAME,
        TEAM_SET_POINTS,
        ANSWER_SET_STATE,
        SET_QUESTION,
        END_GAME,
        DATA_FOR_SCREEN,
        RESET_SERVER,
        KEYPRESS_ON_SCREEN,
        MARK_TEAM,
        SET_BUZZER_LOCK,
        WEBSOCKET_CONNECTION_SUCCESSFUL, // no interface exists for this one!
        UPDATE_MEDIA_STATE,
        REGISTER_BUZZER,
        BUZZER_REGISTERED,
        BUZZER_UNREGISTERED,
        BUZZER_PRESSED,
        LED_ON,
        LED_OFF,
        BUZZER_ARM,
        BUZZER_DISARM,
        WRONG_PACKETTYPE
    };
    static const uint8_t JSON_MAX_LENGTH = 200;
    char* outputStr = nullptr;
    StaticJsonDocument<WsMessage::JSON_MAX_LENGTH> output;
    EPacketTypes packetType;
    char* message = nullptr;

    WsMessage(EPacketTypes);
    WsMessage(EPacketTypes,char*,uint8_t);
    char* JSON();
    ~WsMessage();
};
 
