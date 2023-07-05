#include "handle_server.h"

void setup_server() {
  // WLAN-Verbindung herstellen
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    combined_print("Verbinde mit WLAN....");
  }
  combined_print("Verbunden.");
  // Serververbindung herstellen
  while(!client.connect(serverIP, serverPort)) {
    combined_print("Serververbindung wird aufgebaut");
    delay(1000);
  }
  combined_print("Server verbunden, suche Spiel!");
  last_server_message_timestamp = millis();
  //wait for GAME_CREATED_REPLY
  while (true) {
    DynamicJsonDocument message(512);
    bool received_message = receive_message(&message);
    if (received_message) {

      enum server_message reply_type = message["request"];
      switch (reply_type) {
        case GAME_CREATED_REPLY: {
          combined_print("Spiel erstellt!");
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
  //this code is highly simmilar to message handling in server.py / client.py
  DynamicJsonDocument message(512);
  bool received_message = receive_message(&message);
  if (received_message) {

    enum server_message reply_type = message["request"];
    switch (reply_type) {
      case KEEP_ALIVE_REPLY: {
        break;
      }
      case GAME_CREATED_REPLY: {
        raise_error("GAME_CREATED_REPLY wurde zweimal gesendet!");
      }
      case INFO_REPLY: {
        parse_game_info(&message);
        break;
      }
      case ILLEGAL_MOVE_REPLY: {
        raise_error("Du hast es geschafft einen ungültigen zug zu machen!");
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

//helper to read n bytes from the server into a buffer, because there is no library function provided
void read_message_into_buffer(size_t buffer_size, char* buffer, size_t n_bytes) {
  //n_bytes <= buffer_size
  
  for (int i = 0; i < n_bytes && i < buffer_size; ++i) {
    buffer[i] = client.read();
  }
}

char receive_buffer[max_message_length + 1] = {0};
bool currently_receiving = false;
int message_length = 0;
bool receive_message(DynamicJsonDocument *result) {
  //receive a message from the server
  //has a local state in order to not lose partially received messages
  if (!client.connected() && client.available() < message_length) {
    raise_error(server_connection_lost_error);
  }
  if (!currently_receiving) {
    if (client.available() < message_length_width) { //minimum message length
      return false;
    }
    read_message_into_buffer(max_message_length, receive_buffer, message_length_width);
    void * recv_buff = (void *) receive_buffer;
    message_length = ((int*) receive_buffer)[0];
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
  receive_buffer[message_length] = '\0';
  DeserializationError error = deserializeJson(*result, (const char *) receive_buffer, message_length);
  if (error) {
    Serial.print(message_length);
    Serial.println(receive_buffer);
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    raise_error(invalid_reply_error);
  }
  last_server_message_timestamp = millis();
  message_length = 0;
  return true;
}


char send_buffer[max_message_length + message_length_width] = {0};

void send_message(DynamicJsonDocument message) {
  //send a message to the server
  if (!client.connected()) {
    raise_error(server_connection_lost_error);
  }
  int message_length = serializeJson(message, send_buffer + message_length_width, max_message_length);
  *((int*) send_buffer) = message_length;
  for (size_t i = 0; i < message_length_width; ++i) {
    client.write(send_buffer[i]); //necessary to fix endianness
  }
  client.print(&send_buffer[message_length_width]);
  last_client_message_timestamp = millis();
}

void send_basic_request(enum client_message request) {
  //helper to construct most messages (keep_alive, disconnect ...)
  StaticJsonDocument<16> message;
  message["request"] = request;
  send_message(message);
}

void send_move_request(int move) {
  //constructs and sends a move request to the server
  StaticJsonDocument<32> message;
  message["request"] = MOVE_REQUEST;
  message["move"] = move;
  send_message(message);
}

void make_move(int x, int y) {
  //convert between the different board representations on the server / on the client
  int move = y * board_width + x;
  send_move_request(move);
}

void raise_error [[noreturn]](String error_message) {
  // print the error message, then halt excecution
  combined_print(error_message);
  while(true) {
    ESP.wdtFeed();
  }
}

void forfeit() {
  //forfeits the game and stops the client
  send_basic_request(DISCONNECT_REQUEST);
  game_outcome = lost_outcome;
  game_end_reason = forfeit_reason
  client.stop();
  end_game();
}

void parse_game_info(DynamicJsonDocument *info) {
  //change state in response to info_reply
  JsonObject state = (*info)["state"];
  JsonObject state_status = state["status"];
  JsonArray state_board = state["board"];
  long long client_id = state["client_id"];
  for (size_t y = 0; y < board_height; ++y) {
    for (size_t x = 0; x < board_width; ++x) {
      int index = y * board_width + x;
      if (state_board[index] == nullptr) {
        board[y][x] = 0;
      } else if (state_board[index] == client_id) {
        board[y][x] = 1;
      } else {
        board[y][x] = 2;
      }
    }
  }
  can_move = state["to_move"] == client_id;
  game_outcome = String(state_status["current_state"]);
  game_end_reason = String(state_status["reason"]);
}
