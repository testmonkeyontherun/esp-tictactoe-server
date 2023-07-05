#include "handle_player.h"
#include "handle_sound.h"
#include "splashscreen.h"
void setup_player() {
  // Initialisiere das OLED-Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.drawXBitmap(0, 0, splashscreen +( 2* sizeof(int16_t)), ((int16_t*) ((void *) splashscreen))[0], ((int16_t*) ((void *) splashscreen))[1], WHITE);
  display.display();
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
    if (selected_y < menus[current_menu].height - 1) {
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
    if (selected_x < menus[current_menu].width - 1) {
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
  if (board[selected_y][selected_x] == 0 && can_move) {
    switch_menu(MOVE);
  }
  play_sound_move();
}

void board_b_pressed() {
  switch_menu(FORFEIT);
  play_sound_move();
}

void switch_to_board() {
  switch_menu(BOARD);
}

void try_move() {
  make_move(board_x, board_y);
  board[board_y][board_x] = 1;
  can_move = false;
  play_sound_move();
  switch_to_board();
}

void try_forfeit() {
  forfeit();
  game_end_reason = forfeit_reason;
  game_outcome = lost_outcome;
  end_game();
}


void draw_board(struct menu menu, int selected_x, int selected_y) {
  display.clearDisplay();
  display.drawLine(21, 0, 21, 63, WHITE);
  display.drawLine(42, 0, 42, 63, WHITE);
  display.drawLine(0, 21, 63, 21, WHITE);
  display.drawLine(0, 42, 63, 42, WHITE);
  display.setTextSize(3);

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
  display.setTextSize(1);
  display.setCursor(3*square_width, 0);
  if (can_move) {
    display.println("Zieh!");
  } else {
    display.println("Warte!");
  }
  display.display();
}

void draw_text_menu(struct menu menu, int selected_x, int selected_y) {
  //TODO tune textsize and offset
  const int text_menu_text_size = 1;
  const int text_menu_y_offset = 1;
  const int text_menu_x_offset = 1;
  const int text_menu_text_height = 6;
  display.setTextSize(text_menu_text_size);
  display.clearDisplay();
  for (size_t i = 0; i < menu.height; ++i) {
    int upper_y = i * (text_menu_text_height + text_menu_y_offset * 2);
    int upper_x = text_menu_x_offset;
    if (selected_y == i) {
      //draw active cell
      display.fillRect(upper_x, upper_y, SCREEN_WIDTH - text_menu_x_offset * 2, text_menu_text_height + text_menu_y_offset * 2, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(upper_x, upper_y + text_menu_y_offset);
    display.println(menu.entrys[i].name);
  }
  display.display();
}


void end_game [[noreturn]]() {
  Serial.println("you " + game_outcome + ", because of: " + game_end_reason);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Du hast wegen ");
  display.print(game_end_reason);
  display.print(" ");
  display.print(game_outcome);
  display.println(".");
  display.display();
  if (game_end_reason.compareTo(won_outcome)) {
    play_sound_success();
  } else {
    play_sound_failure();
  }
  delay(10000);
  play_sound_shutdown();
  display.clearDisplay();
  display.display();
  while(true) {
    ESP.wdtFeed();
  }
}
