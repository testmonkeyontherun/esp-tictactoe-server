#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 0
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int square_width = 21;
const int board_width = 3;
const int board_height = 3;

//button related stuff
enum button_indexes {buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonAPin, buttonBPin};
const int button_pins[] = {D7, RX, D6, D5, D4, D3};
const int number_of_buttons = sizeof(button_pins) / sizeof(button_pins[0]);
bool pressed_buttons[number_of_buttons] = {false};
bool new_buttons[number_of_buttons] = {0};

//state
int board[board_height][board_width] = {0};
bool can_move = false;
String game_end_reason = "FEHLER";
String game_outcome = "FEHLER";
String won_outcome = "won";
String lost_outcome = "lost";
String forfeit_reason = "forfeit";

//creating the different menus
enum menu_types {BOARD = 0, MOVE = 1, FORFEIT = 2};
enum menu_types current_menu = BOARD;
int selected_x = 0;
int selected_y = 0;
int board_x = 0;
int board_y = 0;

struct menu_entry {
  void (*a_callback)(void);
  void (*b_callback)(void);
  String name;
};

struct menu {
  struct menu_entry *entrys;
  size_t width;
  size_t height;
  void (*draw)(struct menu, int, int); //draw(struct menu menu, int x, int y)
};

void switch_menu(enum menu_types new_menu);
void board_a_pressed();
void board_b_pressed();
void draw_board(struct menu menu, int selected_x, int selected_y);
struct menu_entry board_entrys[] = {{.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""},
                                    {.a_callback = &board_a_pressed, .b_callback = &board_b_pressed, .name = ""}};

void switch_to_board();
void try_move();
void try_forfeit();
void draw_text_menu(struct menu menu, int selected_x, int selected_y);
struct menu_entry move_entrys[] = {{.a_callback = &switch_to_board, .b_callback = &switch_to_board, .name = "Abbrechen"},
                                   {.a_callback = &try_move, .b_callback = &switch_to_board, .name = "Zug machen"}};

struct menu_entry forfeit_entrys[] = {{.a_callback = &switch_to_board, .b_callback = &switch_to_board, .name = "Abbrechen"},
                                   {.a_callback = &try_forfeit, .b_callback = &switch_to_board, .name = "Aufgeben"}};                     

struct menu menus[] = {[BOARD] = {.entrys = board_entrys, .width = board_width, .height = board_height, .draw = &draw_board},
                       [MOVE] = {.entrys = move_entrys, .width = 1, .height = sizeof(move_entrys) / sizeof(move_entrys[0]), .draw = &draw_text_menu},
                       [FORFEIT] = {.entrys = forfeit_entrys, .width = 1, .height = sizeof(forfeit_entrys) / sizeof(forfeit_entrys[0]), .draw = &draw_text_menu}};

//multitasking
const unsigned long player_polling_interval = 10;
void setup_player();
void handle_player();

void end_game [[noreturn]]();
