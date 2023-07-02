#include "handle_server.h"

void make_move();
void forfeit();

void setup_server() {
  // WLAN-Verbindung herstellen
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Verbinde mit WLAN....");
  }
  Serial.println("Verbunden.");
  // Serververbindung herstellen
  display.setCursor(0,0);
  display.println("Serververbindung wird aufgebaut");
  while(!client.connect(serverIP, serverPort)) {
    //TODO funny animation here ?
    delay(1000);
  }
  last_server_message_timestamp = millis();
  //TODO wait for GAME_CREATED_REPLY
}

void handle_server() {
  //handle server communication
  if (millis() - last_server_poll >= player_polling_interval) {
    last_player_poll = millis();
    //handle incoming messages
    StaticJsonDocument<384> message;
    bool received_message = receive_message(message);
    if (received_message) {

      enum server_message reply_type = message["request"];
      if (reply_type == KEEP_ALIVE_REPLY) {

      } else {
        //TODO handle all types of replys
      }
    }
    if (millis() - last_server_message_timestamp >= server_timeout_time) {
      //TODO handle server disconnect
    }
    //TODO handle outgoing messages
    if (millis() - last_client_message_timestamp >= server_timeout_time / 2) {
      send_basic_request(KEEP_ALIVE_REQUEST);
    }
  }
}
void read_message_into_buffer(size_t buffer_size, char* buffer, size_t n_bytes) {
  //n_bytes <= buffer_size
  
  for (int i = 0; i < n_bytes && i < buffer_size; ++i) {
    buffer[i] = client.read();
  }
}

char receive_buffer[max_message_length] = {0};
bool receive_message(DynamicJsonDocument result) { //TODO
  if (!client.connected()) {
    //TODO ERROR HERE
    return false;
  }
  if (client.available() < message_length_width) { //minimum message length
    return false;
  }
  read_message_into_buffer(max_message_length, receive_buffer, message_length_width);
  int message_length = (int) *receive_buffer;
  //TODO validate if this needs format changing
  if (message_length > max_message_length) {
    //TODO Invalid message
    return false;
  }
  if (client.available() < message_length) {
    //TODO Invalid message
    return false;
  }
  read_message_into_buffer(max_message_length, receive_buffer, message_length);
  DeserializationError error = deserializeJson(result, (const char *) receive_buffer, message_length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  last_server_message_timestamp = millis();
  return true;
}
char send_buffer[max_message_length + message_length_width] = {0};

bool send_message(DynamicJsonDocument message) {
  if (!client.connected()) {
    //TODO raise errors
    return false;
  }
  int message_length = serializeJson(message, send_buffer + message_length_width, max_message_length);
  *((int*) send_buffer) = message_length;
  for (int i = 0; i < message_length + message_length_width; ++i) {
    client.write(send_buffer[i]);
  }
  last_client_message_timestamp = millis();
  return true;
}

bool send_basic_request(enum client_message request) {
  StaticJsonDocument<16> message;
  message["request"] = request;
  return send_message(message);
}

bool send_move_request(int move) {
  StaticJsonDocument<32> message;
  message["request"] = 0;
  message["move"] = move;
  return send_message(message);
}

void aufgeben() {
  // @Jakob

}
