#include "handle_sound.h"

void setup_sound() {
  pinMode(soundPin, OUTPUT);
  sound_is_playing = false;
  sound_start = 0;
  sound_duration = 0;
}

void handle_sound() {
  if (sound_is_playing && (millis() - sound_start >= sound_duration)) {
    analogWrite(soundPin, 0);
    sound_is_playing = false;
  }
}

void play_sound_move() {
  //uses pwm to play a 500hz beep, other sound is not possible during multitasking
  analogWrite(soundPin, 255);
  sound_duration = 200;
  sound_start = millis();
}
//these are possible, because when they are played multitasking is either not yet or no longer running
void play_sound_success() {
  //play_tone(hz, t in ms);
  play_tone(500.0, 250);
  delay(200);
  play_tone(500.0, 250);
  delay(100);
  play_tone(650.0, 500);
}

void play_sound_failure() {
  //play_tone(hz, t in ms);
  play_tone(575.0, 400);
  delay(20);
  play_tone(525.0, 400);
  play_tone(475.0, 500);
}

void play_sound_startup() {
  //play_tone(hz, t in ms);
  play_tone(440.0, 300);
  play_tone(750.0, 300);
}

void play_sound_shutdown() {
  //play_tone(hz, t in ms);
  play_tone(659.255, 600);
  play_tone(440, 600);
}

void play_tone(float frequenz, unsigned long duration) {
  //bit bangs sound into the speaker not multitaskable
  unsigned long start = millis();
  while (millis() - start < duration) {
    digitalWrite(soundPin, HIGH);
    delay(1000.0 / frequenz);
    digitalWrite(soundPin, LOW);
    delay(1000.0 / frequenz);
  }
}
