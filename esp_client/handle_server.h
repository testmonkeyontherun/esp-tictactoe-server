#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <assert.h>
#include "handle_player.h"
//state relevant to server commmunication
const char* ssid =  "esp-server";
const char* password = "BHir8CnjquPkX3";
const char* serverIP = "10.42.0.1";
const int serverPort = 12345;
const unsigned long server_timeout_time = 1000;
unsigned long last_server_message_timestamp = 0;
unsigned long last_client_message_timestamp = 0;
WiFiClient client;

unsigned long server_polling_interval = 10;
unsigned long last_server_poll = 0;
//messages
enum server_message {KEEP_ALIVE_REPLY = 0, INFO_REPLY = 1, GAME_CREATED_REPLY = 2, ILLEGAL_MOVE_REPLY = 3, MOVE_ACCEPTED_REPLY = 4, GAME_ENDED_REPLY = 5};
enum client_message {KEEP_ALIVE_REQUEST = 0, INFO_REQUEST = 1, DISCONNECT_REQUEST = 2, MOVE_REQUEST = 3};
const int message_length_width = 4;
const int max_message_length = 1000;
String server_connection_lost_error = "Serververbindung verloren!";
String invalid_reply_error = "Ungültige Nachricht vom Server!";

void make_move(int x, int y);
void forfeit();
void setup_server();
void handle_server();
void raise_error [[noreturn]](String error_message);
void parse_game_info(JsonObject info);
