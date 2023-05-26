// @file b-end.cpp
/* 
 * Backend-Server Code für das Multiplayer Tic-Tac-Toe Projekt für die BGY21b Projektphase des Fachs 
 * Mikrocontroller an der Carl Benz Schule Koblenz (Gruppe Simon, Jakob, Vincent, David).
*/


// Bibliotheken
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Konstanten
#define ssid = "platzhalter"
#define password = "platzhalter"
#define SERVER_IP = "platzhalter"



// Verbindungsdetails vom Server
IPAddress serverIP(192, 168, 1, SERVER_IP);
int serverPort = 12345;

// Typen von Nachrichten
enum MessageTypes{
    // client -> handler
    KEEP_ALIVE_REQUEST = 0,
    INFO_REQUEST = 1,
    DISCONNECT_REQUEST = 2,
    MOVE_REQUEST = 3,

    // handler -> client
    KEEP_ALIVE_REPLY = 0,
    INFO_REPLY = 1,
    GAME_CREATED_REPlY = 2,
    ILLEGAL_MOVE_REPLY = 3,
    MOVE_ACCEPTED_REPLY = 4,
    GAME_ENDED_REPLY = 5
};

// Struktur für eine Nachricht
struct Message{
    uint32_t magicNumber;
    uint32_t messageLength;
    MessageTypes messageType;
    String messageContent;
};

WifiClient client;



/*
 * Intialisiert Verbindung WiFi -> Server
*/
void setup(){
    Serial.begin(115200);


    // WLAN-Verbindung herstellen
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED){
        delay(1000);
        Serial.println("Verbinde mit WLAN....");
    }
    Serial.println("Verbunden.");

    // Serververbindung herstellen
    if (client.connect(serverIP, serverPort)){
        Serial.println("Verbindung mit Server hergestellt.");
        createGame();
        setInterval(getServerInfo, 1000);
    } else{
        Serial.println("Verbindung zu Server fehlgeschlagen.");
    }
}

/*
 *  Main Loop
*/
void loop(){
    sendKeepAliveRequest();

    if (client.available()){
        Message receivedMessage = recieveMessage();
        processMessage(receivedMessage);
    }
}

// Sendet KeepAlive Anfrage an Server
void sendKeepAliveRequest(){
    Message keepAliveRequest;
    keepAliveRequest.messageLength = sizeof(MessageTypes);
    keepAliveRequest.messageType = KEEP_ALIVE_REQUEST;

    sendMessage(keepAliveRequest);
}

// Erstellt ein neues Spiel beim Server
void createGame(){
    Message gameCreatedReply;
    gameCreatedReply.messageLength = sizeof(MessageTypes);
    gameCreatedReply.messageType = GAME_CREATED_REPLY;
    sendMessage(gameCreatedReply);
}


// Fragt Info vom Server ab
void getServerInfo(){
    Message infoRequest;
    infoRequest.messageLength = sizeof(MessageTypes);
    infoRequest.messageType = INFO_REQUEST;

    sendMessage(infoRequest);
}


// Sendet Zug an Server
// @param[in] move Ist der auszuführende Zug
void sendMove(int move){
    Message moveRequest;
    moveRequest.messageLength = sizeof(MessageTypes) + sizeof(int);
    moveRequest.messageType = MOVE_REQUEST;
    moveRequest.messageContent = String(move);

    sendMessage(moveRequest);
}

/**
 * Verarbeitet empfangende Nachrichten von Server
 * 
 * @param receivedMessage Die Nachricht vom Server 
 */
void processMessage(Message receivedMessage){
    switch(receivedMessage.messageType){
        case KEEP_ALIVE_REPLY:
            //! KEEP_ALIVE_REPLY (muss noch implementieren)
            if (receivedMessage.messageContent == "OK"){
                Serial.write("Verbindung OK....");
            } else {
                Serial.write("Verbindung unterbrochen.");
            }
            break;
        case INFO_REPLY:
            //! INFO_REPLY (muss noch implementieren)
            Serial.print("Server Info erhalten.");
            Serial.print("Informationen: " + receivedMessage.messageContent);

            break;
        case ILLEGAL_MOVE_REPLY:
            // ! fertig implimentieren
            Serial.print("Ungültiger Zug.")

            break;
        case MOVE_ACCEPTED_REPLY:
            // ! MOVE_ACCEPTED_REPLY (muss noch implimentieren)
            Serial.print("Zug akzeptiert.");

            int acceptedMove = recievedMessage.messageContent.toInt();
            Serial.print("Zug " + String(acceptedMove) + "wurde gespielt.");

            break;
        case GAME_ENDED_REPLY:
            // ! GAME_ENDED_REPLY (muss noch implimentieren)
            Serial.print("Spiel beendet.")
        
            break;
        default:
            Serial.println("ERR: Unbekannte Nachricht.")
            break; 
    }
}

/**
 * @brief Sendet Nachricht -> Server
 * 
 * @param message Zu sendende Nachricht
 */
void sendMessage(Message message){
    client.write((const uint8_t*)&message, sizeof(Message));
}

/**
 * @brief Empfängt Nachricht von Server
 * 
 * @return recievedMessage Empfangende Nachricht
 */
Message recieveMessage(){
    Message receivedMessage;
    client.read((uint8_t*)&receivedMessage, sizeof(Message));
    return receivedMessage;
}
