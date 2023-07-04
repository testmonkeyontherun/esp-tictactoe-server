#pragma once
const int soundPin = D8;
unsigned long sound_start;
unsigned long sound_duration;
unsigned long sound_is_playing;
unsigned long sound_polling_interval = 1;

void play_sound_success(); //TODO
void play_sound_failure();
void play_sound_startup();
void play_sound_shutdown();
void play_sound_move();
void setup_sound();
void handle_sound();
