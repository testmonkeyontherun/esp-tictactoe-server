#include "handle_sound.h"

void setup_sound() {
  pinMode(soundPin, OUTPUT);
  sound_is_playing = false;
  sound_start = 0;
  sound_duration = 0;
}

void handle_sound() {
  Serial.print(millis());
  Serial.println("handle_sound");
  if (sound_is_playing && millis() - sound_start >= sound_duration) {
    analogWrite(soundPin, 0);
    sound_is_playing = false;
  }
}

void play_sound_move() {
  analogWrite(soundPin, 255);
  sound_duration = 2000;
  sound_start = millis();
}

void play_sound_success() {
  //play_tone(hz, t in ms);
  //TODO come up with jingle
  play_tone(750.0, 1000);
  play_tone(750.0, 1000);
  play_tone(1000.0, 3000);
}

void play_sound_failure() {
  //play_tone(hz, t in ms);
  //TODO come up with jingle
  play_tone(750.0, 1000);
  play_tone(750.0, 1000);
  play_tone(1000.0, 3000);
}

void play_sound_startup() {
  //play_tone(hz, t in ms);
  //TODO come up with jingle
  play_tone(750.0, 1000);
  play_tone(750.0, 1000);
  play_tone(1000.0, 3000);
}

void play_sound_shutdown() {
  //play_tone(hz, t in ms);
  //TODO come up with jingle
  play_tone(750.0, 1000);
  play_tone(750.0, 1000);
  play_tone(1000.0, 3000);
}

void play_tone(float frequenz, unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    digitalWrite(soundPin, HIGH);
    delay(1000.0 / frequenz);
    digitalWrite(soundPin, LOW);
    delay(1000.0 / frequenz);
  }
}
