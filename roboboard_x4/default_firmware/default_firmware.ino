#include <Arduino.h>
/*
  Default firmware shipped in RoboBoard X4.
  Contains empty Arduino sketch with enabled features:
  - Totem App connectivity
  - Motor beep during power on and app connect
  - RGB blink during power on and app connect
*/
// Function called after loading configuration from memory
// and before system is initialized (with loaded configuration)
// Note: if "initRoboBoard()" is used - overrides board parameters
// set in Arduino IDE Tools menu.
void initRoboBoard() {
  // Enable board status indication with RGB LED
  // - displays battery state on power on
  // - blink red if restarted due low battery
  // - displays Totem App connection state
  Board.setStatusRGB(true);
  // Enable board status indication with motor beep sounds
  // - beep when board is powered up
  // - beep on Totem App connect / disconnect
  Board.setStatusSound(true);
}
// Initialize program
void setup() {
  // Enable TotemApp connectivity
  TotemApp.begin();
}
// Loop program
void loop() {

}
