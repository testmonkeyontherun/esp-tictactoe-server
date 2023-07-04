/* 
 * Backend-Server Code für das Multiplayer Tic-Tac-Toe Projekt für die BGY21b Projektphase des Fachs 
 * Mikrocontroller an der Carl Benz Schule Koblenz (Gruppe Simon, Jakob, Vincent, David).
*/
#include "handle_server.h"
#include "handle_player.h"
#include "handle_sound.h"

struct task {
  void (*task_function) (void);
  unsigned long call_interval_in_millis;
  unsigned long last_call_timestamp;
};

void multitask(struct task *tasks, size_t n_tasks);

void setup(){
  Serial.begin(115200);
  //tasks erstellen
  struct task handle_player_task = {.task_function = handle_player, .call_interval_in_millis = player_polling_interval, .last_call_timestamp = 0};
  struct task handle_server_task = {.task_function = &handle_server, .call_interval_in_millis = server_polling_interval, .last_call_timestamp = 0};
  struct task handle_sound_task = {.task_function = &handle_sound, .call_interval_in_millis = sound_polling_interval, .last_call_timestamp = 0};
  struct task tasks[] = {handle_server_task, handle_player_task, handle_sound_task};
  size_t n_tasks = sizeof(tasks) / sizeof(tasks[0]);
  setup_sound();
  setup_player();
  setup_server();
  combined_print("Spiel erstellt!");
  multitask(tasks, n_tasks);
}

void multitask(struct task *tasks, size_t n_tasks) {
  while (true) {
    for (size_t i = 0; i < n_tasks; ++i) {
      ESP.wdtFeed();
      if (millis() - tasks[i].last_call_timestamp >= tasks[i].call_interval_in_millis) {
        tasks[i].last_call_timestamp = millis();
        tasks[i].task_function();
      }
    }
  }
}

void loop() {
  //intentionally left blank, once this part is reached the code is over
}

void combined_print(String to_print) {
  Serial.println(to_print);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(to_print);
}