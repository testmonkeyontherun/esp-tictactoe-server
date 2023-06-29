/* 
 * Backend-Server Code für das Multiplayer Tic-Tac-Toe Projekt für die BGY21b Projektphase des Fachs 
 * Mikrocontroller an der Carl Benz Schule Koblenz (Gruppe Simon, Jakob, Vincent, David).
*/


// Bibliotheken
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//state relevant to menu navigation
  // Pins für die Knöpfe
const int number_of_buttons = 6;
enum button_indexes {buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonAPin, buttonBPin};
const int button_pins[number_of_buttons] = {2, 3, 4, 5, 6, 7};
  //rest
enum current_menu {BOARD, MOVE, FORFEIT};
bool pressed_buttons[number_of_buttons] = {false};
bool new_buttons[number_of_buttons] = {0};
unsigned long player_polling_interval = 100;
unsigned long last_player_poll = 0;
enum current_menu current_menu = BOARD;
int selectedX = 0;
int selectedY = 0;
int submenu_option = 0;
int square_width = 21;
int board[3][3] = {{0}, {0}, {0}};

//state relevant to server commmunication
const char* ssid =  "platzhalter";
const char* password = "platzhalter";
IPAddress serverIP(192, 168, 3, 125);
int serverPort = 12345;
WiFiClient client;

unsigned long server_polling_interval = 10;
unsigned long last_server_poll = 0;
  //messages
  enum server_message {KEEP_ALIVE_REPLY = 0, INFO_REPLY = 1, GAME_CREATED_REPLY = 2, ILLEGAL_MOVE_REPLY = 3, MOVE_ACCEPTED_REPLY = 4, GAME_ENDED_REPLY = 5};
  enum client_message {KEEP_ALIVE_REQUEST = 0, INFO_REQUEST = 1, DISCONNECT_REQUEST = 2, MOVE_REQUEST = 3};
  const int message_length_width = 4;
  const int max_message_length = 1000;
void play_sound_success(); //TODO
void play_sound_failure();
void play_sound_move();
void make_move();
void forfeit();



void read_message_into_buffer(size_t buffer_size, char* buffer, size_t n_bytes) {
  //n_bytes <= buffer_size
  
  for (int i = 0; i < n_bytes && i < buffer_size; ++i) {
    buffer[i] = client.read();
  }
}

char receive_buffer[max_message_length] = {0};
bool receive_message(JsonDocument result) { //TODO
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
  deserializeJson(result, (const char *) receive_buffer, message_length);
  return true;
}
char send_buffer[max_message_length + message_length_width] = {0};
bool send_message(JsonDocument message) {
  if (!client.connected()) {
    //TODO raise errors
    return false;
  }
  int message_length = serializeJson(message, send_buffer + message_length_width, max_message_length);
  *((int*) send_buffer) = message_length;
  for (int i = 0; i < message_length + message_length_width; ++i) {
    client.write(send_buffer[i]);
  }
  return true;
}

void send_keep_alive_message() {

}

/*
 * Intialisiert Verbindung WiFi -> Server
*/
void setup(){
    // Initialisiere das OLED-Display
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);

    // Initialisiere die Knöpfe
    for (size_t button = 0; button < number_of_buttons; ++button) {
      pinMode(button_pins[button], INPUT_PULLUP);
    }
    Serial.begin(115200);

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
    //wait for GAME_CREATED_REPLY
}

//state relevant to server_communication
void loop(){
  //handle button inputs, display
  if (millis() - last_player_poll >= player_polling_interval) {
    
    last_player_poll = millis();
    //check for new button presses since last cycle
    for (size_t button = 0; button < number_of_buttons; ++button) {
        new_buttons[button] = false;
        if (digitalRead(button_pins[button]) == HIGH) {
          if (pressed_buttons[button] == false) {
            new_buttons[button] = true;
          }
          pressed_buttons[button] = true;
        } else {
          pressed_buttons[button] = false;
        }
    }
    //menu navigation
    if (current_menu == BOARD) {
      if (new_buttons[buttonBPin]) {
        current_menu = FORFEIT;
        submenu_option = 0;
        play_sound_success();
      } else if (new_buttons[buttonAPin]) {
        if (board[selectedY][selectedX] == 0) { //TODO official game representation
          current_menu = MOVE;
          submenu_option = 0;
          play_sound_success();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonUpPin]) {
        if (selectedY > 0) {
          --selectedY;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonDownPin]) {
        if (selectedY < 2) {
          ++selectedY;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonLeftPin]) {
        if (selectedX > 0) {
          --selectedX;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonRightPin]) {
        if (selectedX < 2) {
          ++selectedX;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      }
    } else if(current_menu == MOVE) {
      if (new_buttons[buttonBPin]) {
        current_menu = BOARD;
        play_sound_success();
      } else if (new_buttons[buttonAPin]) {
        if (submenu_option == 0) {
          current_menu = BOARD;
        } else if(submenu_option == 1) {
          make_move(); //TODO
        }
        play_sound_success();
      } else if (new_buttons[buttonUpPin]) {
        if (submenu_option == 0) {
          submenu_option = 1;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonDownPin]) {
        if (submenu_option == 1) {
          submenu_option = 0;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      }
    } else if (current_menu == FORFEIT) {
      if (new_buttons[buttonBPin]) {
        current_menu = BOARD;
      } else if (new_buttons[buttonAPin]) {
        if (submenu_option == 0) {
          current_menu = BOARD;
        } else if(submenu_option == 1) {
          forfeit(); //TODO
        }
        play_sound_success();
      } else if (new_buttons[buttonUpPin]) {
        if (submenu_option == 0) {
          submenu_option = 1;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      } else if (new_buttons[buttonDownPin]) {
        if (submenu_option == 1) {
          submenu_option = 0;
          play_sound_move();
        } else {
          play_sound_failure();
        }
      }
    }
    //draw board / menus
    display.clearDisplay();
    if (current_menu == BOARD) {
        display.drawLine(21, 0, 21, 64, WHITE);
        display.drawLine(42, 0, 42, 64, WHITE);
        display.drawLine(0, 21, 0, 64, WHITE);
        display.drawLine(0, 42, 0, 64, WHITE);
        //todo textgröße hier anpassen////

        for (int x = 0; x < 3; ++x) {
          for (int y = 0; y < 3; ++y) {
            int upper_x = x * square_width;
            int upper_y = y * square_width;
            if (x == selectedX && y == selectedY) {
              //draw active cell
              display.fillRect(upper_x, upper_y, square_width, square_width, WHITE);
              display.setTextColor(BLACK);
            } else {
              display.setTextColor(WHITE);
            }
            if (board[y][x] == 1) {
              //draw x
              display.setCursor(upper_x, upper_y);
              display.println("X");
            } else if (board[y][x] == 2) {
              //draw o
              display.setCursor(upper_x, upper_y);
              display.println("O");
            }
          }
        }
        
    } else if (current_menu == FORFEIT) {
      //TODO draw forfeit
    } else if (current_menu == MOVE) {
      //TODO draw move
    }
  }
  //handle server communication
  if (millis() - last_server_poll >= player_polling_interval) {
    last_player_poll = millis();
    //TODO alles andere
  }
}
