#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins für die Knöpfe
const int number_of_buttons = 6;
enum button_indexes {buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonAPin, buttonBPin};
const int button_pins[number_of_buttons] = {2, 3, 4, 5, 6, 7};
//rest
enum current_menu {BOARD, MOVE, FORFEIT};
bool pressed_buttons[number_of_buttons] = {false};
bool new_buttons[number_of_buttons] = {0};
const unsigned long player_polling_interval = 100;
unsigned long last_player_poll = 0;
enum current_menu current_menu = BOARD;
int selectedX = 0;
int selectedY = 0;
int submenu_option = 0;
const int square_width = 21;
int board[3][3] = {{0}, {0}, {0}};

void setup_player();
void handle_player();