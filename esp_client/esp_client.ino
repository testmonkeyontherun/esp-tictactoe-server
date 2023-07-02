/* 
 * Backend-Server Code für das Multiplayer Tic-Tac-Toe Projekt für die BGY21b Projektphase des Fachs 
 * Mikrocontroller an der Carl Benz Schule Koblenz (Gruppe Simon, Jakob, Vincent, David).
*/


// Bibliotheken
struct task {
  void (*task_function) (void);
  unsigned long call_interval_in_millis;
  unsigned long last_call_timestamp;
};

void multitask(struct task *tasks, size_t n_tasks);

void handle_server(void);
void handle_player(void);
void handle_sound(void);

void setup(){
  Serial.begin(115200);
  //tasks erstellen
  struct task handle_player_task = {.task_function = &handle_player, .call_interval_in_millis = player_polling_interval, .last_call_timestamp = 0};
  struct task handle_server_task = {.task_function = &handle_server, .call_interval_in_millis = server_polling_interval, .last_call_timestamp = 0};
  struct task handle_sound_task = {.task_function = &handle_sound, .call_interval_in_millis = sound_polling_interval, .last_call_timestamp = 0};
  struct task tasks[] = {handle_server_task, handle_player_task, handle_sound_task};
  size_t n_tasks = sizeof(tasks) / sizeof(tasks[0]);
  setup_player();
  setup_sound();
  setup_server();
  multitask(tasks, n_tasks);
}

void multitask(struct task *tasks, size_t n_tasks) {
  while (true) {
    for (size_t i = 0; i < n_tasks; ++i) {
      if (millis() - tasks[i].last_call_timestamp >= tasks[i].call_interval_in_millis) {
        tasks[i].last_call_timestamp = millis();
        tasks[i]->task_function();
      }
    }
  }
}
