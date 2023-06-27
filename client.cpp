/* 
 * Backend-Server Code für das Multiplayer Tic-Tac-Toe Projekt für die BGY21b Projektphase des Fachs 
 * Mikrocontroller an der Carl Benz Schule Koblenz (Gruppe Simon, Jakob, Vincent, David).
*/


// Bibliotheken
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Arduinojson.h>

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
enum button_indexes {buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonAPin, buttonBpin};
const int button_pins[number_of_buttons] = {2, 3, 4, 5, 6, 7};
  //rest
enum current_menu {BOARD, MOVE, FORFEIT}
bool pressed_buttons[number_of_buttons] = {0};
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
  // Konstanten
#define ssid = "platzhalter"
#define password = "platzhalter"
#define SERVER_IP = "platzhalter"
  // Verbindungsdetails vom Server
IPAddress serverIP(192, 168, 1, SERVER_IP);
int serverPort = 12345;
WifiClient client;

unsigned long server_polling_interval = 10;
unsigned long last_server_poll = 0;


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
    if (client.connect(serverIP, serverPort)){
        Serial.println("Verbindung mit Server hergestellt.");
        createGame();
        setInterval(getServerInfo, 1000);
    } else{
        Serial.println("Verbindung zu Server fehlgeschlagen.");
    }
}




//state relevant to server_communication
void loop(){
  //handle button inputs, display
  if (millis() - last_player_poll >= player_polling_interval) {
    
    last_player_poll = millis();
    //check for new button presses since last cycle
    for (size_t button = 0; button < number_of_buttons; ++button) {
        new_buttons[button] = false;
        if digitaRead(button_pins[button]) == high) {
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
    if (current_menu == board) {
        display.drawLine(21, 0, 21, 64, WHITE);
        display.drawLine(42, 0, 42, 64, WHITE);
        display.drawLine(0, 21, 0, 64, WHITE);
        display.drawLine(0, 42, 0, 64, WHITE);

        for (int x = 0; x < 3; ++x) {
          for (int y = 0; y < 3; ++y) {
            if (x == selectedX && y = selectedY) {
              //draw active cell
              int upper_x = x * square_width;
              int lower_x = upper_x + square_width;
              int upper_y = y * square_width;
              int lower_y = upper_y + square_width;
              display.fillrect(upper_x, upper_y, lower_x, lower_y);
              
            } else if (board[y][x] == 1) {
              //draw x 
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
  if (millis() - last_server_poll >= player_server_interval) {
    last_player_poll = millis()
    //TODO alles andere
  }
}
