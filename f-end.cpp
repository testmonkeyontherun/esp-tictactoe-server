#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins für die Knöpfe
const int buttonUpPin = 2;
const int buttonDownPin = 3;
const int buttonLeftPin = 4;
const int buttonRightPin = 5;
const int buttonSelectPin = 6;

// Menüoptionen
enum MenuOption {
  MAIN_MENU,
  GAME_MENU
};

MenuOption currentMenu = MAIN_MENU;
int selectedGame = 0;
int selectedX = 0;
int selectedY = 0;

void setup() {
  // Initialisiere das OLED-Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Initialisiere die Knöpfe
  pinMode(buttonUpPin, INPUT_PULL);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonSelectPin, INPUT_PULLUP);
}

void loop() {
  switch (currentMenu) {
    case MAIN_MENU:
      displayMainMenu();
      break;

    case GAME_MENU:
      displayGameMenu();
      break;
  }

  // Überprüfe die Knöpfe und aktualisiere die Auswahl
  checkButtons();

  // Führe den Zug aus, wenn die Auswahl bestätigt wird
  if (digitalRead(buttonSelectPin) == LOW) {
    if (currentMenu == MAIN_MENU) {
      // Übergehe zum Spielmenü
      currentMenu = GAME_MENU;
    } else if (currentMenu == GAME_MENU) {
      // Sende den Spielzug an den Server und verarbeite die Antwort
      sendMoveToServer(selectedGame, selectedX, selectedY);
      processServerResponse();
    }

    // Warte, bis der Knopf losgelassen wird, um mehrere Aktionen zu vermeiden
    while (digitalRead(buttonSelectPin) == LOW) {
      delay(10);
    }
  }
}

void displayMainMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Hauptmenü");
  // Hier kannst du weitere Optionen für das Hauptmenü hinzufügen
  display.display();
}

void displayGameMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Spielmenü");
  display.print("Spiel: ");
  display.println(selectedGame);
  display.print("Position: X=");
  display.print(selectedX);
  display.print(", Y=");
  display.println(selectedY);
  display.display();
}

void checkButtons() {
  // Überprüfe den Knopf zum Hochscrollen
  if (digitalRead(buttonUpPin) == LOW) {
    if (currentMenu == MAIN_MENU) {
      // Implementiere die Logik zum Hochscrollen im Hauptmenü
    } else if (currentMenu == GAME_MENU) {
      if (selectedGame < maxGameCount - 1) {
        selectedGame++;
      }
    }

    delay(200); // Verzögerung, um mehrere Aktionen zu vermeiden
  }

  // Überprüfe den Knopf zum Herunterscrollen
  if (digitalRead(buttonDownPin) == LOW) {
    if (currentMenu == MAIN_MENU) {
      // Implementiere die Logik zum Herunterscrollen im Hauptmenü
    } else if (currentMenu == GAME_MENU) {
      if (selectedGame > 0) {
        selectedGame--;
      }
    }

    delay(200); // Verzögerung, um mehrere Aktionen zu vermeiden
  }

  // Überprüfe den Knopf zum Linksscrollen
  if (digitalRead(buttonLeftPin) == LOW) {
    if (currentMenu == MAIN_MENU) {
      // Implementiere die Logik zum Linksscrollen im Hauptmenü
    } else if (currentMenu == GAME_MENU) {
      if (selectedX > 0) {
        selectedX--;
      }
    }

    delay(200); // Verzögerung, um mehrere Aktionen zu vermeiden
  }

  // Überprüfe den Knopf zum Rechtsscrollen
  if (digitalRead(buttonRightPin) == LOW) {
    if (currentMenu == MAIN_MENU) {
      // Implementiere die Logik zum Rechtsscrollen im Hauptmenü
    } else if (currentMenu == GAME_MENU) {
      if (selectedX < maxX - 1) {
        selectedX++;
      }
    }

    delay(200); // Verzögerung, um mehrere Aktionen zu vermeiden
  }
}

void sendMoveToServer(int game, int x, int y) {
  // Implementiere den Code zum Senden des Spielzugs an den Server
}

void processServerResponse() {
  // Implementiere den Code zum Verarbeiten der Serverantwort
}
