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
    }
  }
  if (new_buttons[buttonDonwPin]) {
    if (selected_y < menus[current_menu].height) {
      ++selected_y;
    }
  }
  if (new_buttons[buttonLeftPin]) {
    if (selected_x > 0) {
      --selected_x;
    }
  }
  if (new_buttons[buttonRightPin]) {
    if (selected_x < menus[current_menu].width) {
      ++selected_x;
    }
  }
  size_t current_selection_index = selected_y * width + selected_x
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

void handle_player_bak() {
  
  //menu navigation
  if (current_menu == BOARD) {
    if (new_buttons[buttonBPin]) {
      current_menu = FORFEIT;
      submenu_option = 0;
      play_sound_success();
    } else if (new_buttons[buttonAPin]) {
      if (board[selectedY][selectedX] == 0) {
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
    const char* menuOptions[NUM_Options] = {"JA", "NEIN"};
    int selectedOption = 0;
    for (int x = 0; x < 2; x++) {
      if (x == selectedOption) {
        display.print("→");
      } else {
        display.print(" ");
      }
      display.println(menuOptions[x]);

    }
    display.display();

    if(digitalRead(buttonLeftPin) == LOW) {
      selectedOption = (selectedOption + NUM_Options - 1) % NUM_Options;
      delay(/*hier noch einfügen, da SKT unklar*/);
    } else if (digitalRead(buttonRightPin) == LOW) {
      selectedOption = (selectedOption + 1) % NUM_Options;
      delay(/*hier noch einfügen, da SKT unklar*/);
    } else if (digitalRead(buttonAPin) == LOW) {

      if(selectedOption == 0) {
        //Aktion für JA - Verbindung trennen?
        
      } else if (selectedOption == 1) {
        //aktion für Nein - zurück zum Spielboard
      }

      delay(/*hier noch einfügen, da SKT unklar*/);

    }


  } else if (current_menu == MOVE) {
    const char* menuOptionsForMove[MOVEMENU_OPTIONS] = {"JA", "NEIN"};
    int selectedMoveOption = 0;
    bool MoveLoggedIn = false;

    for (int x = 0; x < MOVEMENU_OPTIONS; i++) {
      if(i == selectedOption) {
        display.print("→");
      } else {
        display.print(" ");
      }
      display.println(menuOptionsForMove[x]);
    }

    display.display();

    if (digitalRead(buttonLeftPin) == LOW) {

      selectedMoveOption = (selectedMoveOption + MOVEMENU_OPTIONS - 1) % MOVEMENU_OPTIONS;
      delay(/*hier noch einfügen, da SKT unklar*/);

    } else if (digitalRead(buttonRightPin) == LOW) {

      selectedMoveOption = (selectedMoveOption + MOVEMENU_OPTIONS + 1) % NUM_Options;
      delay(/*hier noch einfügen, da SKT unklar*/);
    
    } else if (digitalRead(buttonDownPin) == LOW) {
      
      if (selectedMoveOption == 0) {

        MoveLoggedIn = true;
      } else if (selectedMoveOption == 1) {
        // Zug nicht einloggen - zurück zum spielboard
      }

      delay(/*hier noch einfügen, da SKT unklar*/);
      // Spiel fortsetzen


    } 
  }
}