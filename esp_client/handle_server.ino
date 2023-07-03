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
  //wait for GAME_CREATED_REPLY
  //TODO some sort of feedback here
  while (true) {
    StaticJsonDocument<384> message;
    bool received_message = receive_message(message);
    if (received_message) {

      enum server_message reply_type = message["request"];
      switch (reply_type) {
        case GAME_CREATED_REPLY: {
          return;
        }
        case KEEP_ALIVE_REPLY: {
          break;
        }
        default: {
          raise_error("Unerwartete Nachricht vom Server!");
        }
      }
    }
    if (millis() - last_server_message_timestamp >= server_timeout_time) {
    raise_error(server_connection_lost_error);
    }
    if (millis() - last_client_message_timestamp >= server_timeout_time / 2) {
      send_basic_request(KEEP_ALIVE_REQUEST);
    }
  }
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
        parse_game_info(message["state"]);
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
        end_game();
      }
      default: {
        raise_error(invalid_reply_error);
      }
    }
  }
  if (millis() - last_server_message_timestamp >= server_timeout_time) {
    raise_error(server_connection_lost_error);
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
bool currently_receiving = false;
int message_length = 0;
bool receive_message(DynamicJsonDocument result) {
  if (!client.connected()) {
    raise_error(server_connection_lost_error);
  }
  if (!currently_receiving) {
    if (client.available() < message_length_width) { //minimum message length
      return false;
    }
    read_message_into_buffer(max_message_length, receive_buffer, message_length_width);
    message_length = (int) *receive_buffer;
    //TODO validate if this needs format changing
    if (message_length > max_message_length) {
      raise_error(invalid_reply_error);
    }
  }
  if (client.available() < message_length) {
    currently_receiving = true;
    return false;
  }
  currently_receiving = false;
  read_message_into_buffer(max_message_length, receive_buffer, message_length);
  DeserializationError error = deserializeJson(result, (const char *) receive_buffer, message_length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    raise_error(invalid_reply_error);
  }
  last_server_message_timestamp = millis();
  return true;
}
char send_buffer[max_message_length + message_length_width] = {0};

void send_message(DynamicJsonDocument message) {
  if (!client.connected()) {
    raise_error(server_connection_lost_error);
  }
  int message_length = serializeJson(message, send_buffer + message_length_width, max_message_length);
  *((int*) send_buffer) = message_length;
  for (int i = 0; i < message_length + message_length_width; ++i) {
    client.write(send_buffer[i]);
  }
  last_client_message_timestamp = millis();
}

void send_basic_request(enum client_message request) {
  StaticJsonDocument<16> message;
  message["request"] = request;
  send_message(message);
}

void send_move_request(int move) {
  StaticJsonDocument<32> message;
  message["request"] = 0;
  message["move"] = move;
  send_message(message);
}

void make_move(int x, int y) {
  int move = y * board_width + x;
  send_move_request(move);
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

void parse_game_info(JsonObject info) {
  int client_id = info["client_id"];
  for (size_t y = 0; y < board_height; ++y) {
    for (size_t x = 0; y < board_width; ++x) {
      int index = y * board_width + x;
      if (info["board"][index].isNull()) {
        board[y][x] = 0;
      } else if (info["board"][index] == client_id) {
        board[y][x] = 1;
      } else {
        board[y][x] = 2;
      }
    }
  }
  can_move = info["to_move"] == client_id;
  if (!strcmp(info["status"]["current_state"], "playing")) {
    game_outcome = String(info["status"]["current_state"]);
    game_end_reason = String(info["status"]["reason"]);
  }
}