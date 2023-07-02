void play_sound_success(); //TODO
void play_sound_failure();
void play_sound_move();

const int analogOutPin = D7;

void setup() {
  // play_sound_move();
  pinMode(analogOutPin, OUTPUT);
  play_sound_success();
}
unsigned long sound_start;
unsigned long sound_duration;

void play_sound_move() {


  analogWrite(analogOutPin, 255);
  sound_duration = 2000;
  sound_start = millis();
}

void play_sound_success() {
  //play_tone(hz, t in ms);
  play_tone(750.0, 1000);
  delay(500);
  play_tone(750.0, 1000);
  delay(500);
  play_tone(1000.0, 3000);
}
void play_tone(float frequenz, int duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    digitalWrite(analogOutPin, HIGH);
    delay(1000.0 / frequenz);
    digitalWrite(analogOutPin, LOW);
    delay(1000.0 / frequenz);
  }
}

void loop() {


  if (millis() - sound_start >= sound_duration) {
    analogWrite(analogOutPin, 0);
    delay(1000);

  }
}

/*void play_sound_success() {

  analogWrite(analogOutPin, 255);
  delay(2000);
  analogWrite(analogOutPin, 128);
  delay(1000);
  }*/
