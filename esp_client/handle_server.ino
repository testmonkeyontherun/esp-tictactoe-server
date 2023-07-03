#include "handle_server.h"

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
  //handle incoming messages
  StaticJsonDocument<384> message;
  bool received_message = receive_message(message);
  if (received_message) {

    enum server_message reply_type = message["request"];
    switch (reply_type) {
      case KEEP_ALIVE_REPLY: {
        break;
      }
      case GAME_CREATED_REPLY: {
        raise_error("GAME_CREATED_REPLY was sent twice!");
      }
      case INFO_REPLY: {
        //TODO
        break;
      }
      case ILLEGAL_MOVE_REPLY: {
        raise_error("You managed to make an illegal move!");
        break;
      }
      case MOVE_ACCEPTED_REPLY: {
        break;
      }
      case GAME_ENDED_REPLY: {
        //TODO
        break;
      }
      default: {
        raise_error("Invalid reply!");
      }
    }
  }
  if (millis() - last_server_message_timestamp >= server_timeout_time) {
    raise_error("Server connection lost!");
  }
  if (millis() - last_client_message_timestamp >= server_timeout_time / 2) {
    send_basic_request(KEEP_ALIVE_REQUEST);
  }
}
void read_message_into_buffer(size_t buffer_size, char* buffer, size_t n_bytes) {
  //n_bytes <= buffer_size
  
  for (int i = 0; i < n_bytes && i < buffer_size; ++i) {
    buffer[i] = client.read();
  }
}

char receive_buffer[max_message_length] = {0};
bool receive_message(DynamicJsonDocument result) { //TODO keep track of buffer over multiple calls, 
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

bool make_move(int x, int y) {
  return false; //TODO
}

void raise_error [[noreturn]](String error_message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(error_message);
  while(true);
}

void forfeit() {
  send_basic_request(DISCONNECT_REQUEST);
  client.stop();
}
