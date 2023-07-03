#include "handle_player.h"
#include "handle_sound.h"
void setup_player() {
  // Initialisiere das OLED-Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  //TODO startup animation
  play_sound_startup();

  // Initialisiere die Knöpfe
  for (size_t button = 0; button < number_of_buttons; ++button) {
    pinMode(button_pins[button], INPUT_PULLUP);
  }
}

void handle_player() {
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
  if (new_buttons[buttonUpPin]) {
    if (selected_y > 0) {
      --selected_y;
      play_sound_move();
    }
  }
  if (new_buttons[buttonDownPin]) {
    if (selected_y < menus[current_menu].height) {
      ++selected_y;
      play_sound_move();
    }
  }
  if (new_buttons[buttonLeftPin]) {
    if (selected_x > 0) {
      --selected_x;
      play_sound_move();
    }
  }
  if (new_buttons[buttonRightPin]) {
    if (selected_x < menus[current_menu].width) {
      ++selected_x;
      play_sound_move();
    }
  }
  size_t current_selection_index = selected_y * menus[current_menu].width + selected_x;
  //has to be only computed once despite menu switching, because only one selection is valid at a time.
  struct menu_entry current_entry = menus[current_menu].entrys[current_selection_index];
  if (new_buttons[buttonAPin]) {
    current_entry.a_callback();
  } else if (new_buttons[buttonBPin]) {
    current_entry.b_callback();
  }
  //drawing
  menus[current_menu].draw(menus[current_menu], selected_x, selected_y);
}

void switch_menu(enum menu_types new_menu) {
  current_menu = new_menu;
  if (new_menu == BOARD) {
    selected_x = board_x;
    selected_y = board_y;
  } else {
    board_x = selected_x;
    board_y = selected_y;
    selected_x = 0;
    selected_y = 0;
  }
}

void board_a_pressed() {
  if (board[selected_y][selected_x] == 0) {
    switch_menu(MOVE);
    play_sound_success();
  } else {
    play_sound_failure();
  }
}

void board_b_pressed() {
  switch_menu(FORFEIT);
  play_sound_success();
}

void switch_to_board() {
  switch_menu(BOARD);
  play_sound_success();
}

void try_move() {
  make_move(board_x, board_y);
  play_sound_success();
  board[board_x][board_y] = 1;
  switch_to_board();
}

void try_forfeit() {
  forfeit();
  display.clearDisplay();
  //TODO maybe animation
  play_sound_shutdown();
  loop();
}


void draw_board(struct menu menu, int selected_x, int selected_y) {
  display.clearDisplay();
  display.drawLine(21, 0, 21, 64, WHITE);
  display.drawLine(42, 0, 42, 64, WHITE);
  display.drawLine(0, 21, 0, 64, WHITE);
  display.drawLine(0, 42, 0, 64, WHITE);
  //todo textgröße hier anpassen////

  for (int x = 0; x < 3; ++x) {
    for (int y = 0; y < 3; ++y) {
      int upper_x = x * square_width;
      int upper_y = y * square_width;
      if (x == selected_x && y == selected_y) {
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
}

void draw_text_menu(struct menu menu, int selected_x, int selected_y) {
  //TODO tune textsize and offset
  const int text_menu_text_size = 10;
  const int text_menu_y_offset = 1;
  const int text_menu_x_offset = 1;
  display.setTextSize(text_menu_text_size);
  display.clearDisplay();
  for (size_t i = 0; i < menu.height; ++i) {
    int upper_y = i * (text_menu_text_size + text_menu_y_offset * 2);
    int upper_x = text_menu_x_offset;
    if (selected_y == i) {
      //draw active cell
      display.fillRect(upper_x, upper_y, SCREEN_WIDTH - text_menu_x_offset * 2, text_menu_text_size + text_menu_y_offset * 2, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(upper_x, upper_y + text_menu_y_offset);
    display.println(menu.entrys[i].name);
  }
}
