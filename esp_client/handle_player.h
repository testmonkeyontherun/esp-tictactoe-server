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
bool pressed_buttons[number_of_buttons] = {false};
bool new_buttons[number_of_buttons] = {0};

const unsigned long player_polling_interval = 100;
const int square_width = 21;
int board[3][3] = {{0}, {0}, {0}};

enum menu_types {BOARD = 0, MOVE = 1, FORFEIT = 2};
enum current_menu current_menu = BOARD;
int selected_x = 0;
int selected_y = 0;

struct menu_entry {
  void (*a_callback)(void);
  void (*b_callback)(void);
  char* name;
};

struct menu {
  struct menu_entry *entrys;
  size_t n_entrys;
  size_t width;
  size_t height;
  void (*draw)(struct menu, int, int); //draw(struct menu menu, int x, int y)
}



struct menu menus[] = {}

void switch_menu(enum menu_types new_menu);

void setup_player();
void handle_player();