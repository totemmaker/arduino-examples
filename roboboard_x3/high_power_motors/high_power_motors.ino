#include <Arduino.h>
/* DESCRIPTION:
 * Use RoboBoard X3 with more powerful motors. This code retains all Totem App
 * controls and prevents battery shutoff due high power usage.
 *
 * Values of "DC.setAccelerationTime()" and "DC.setRange()" can be
 * changed to limit motor speed and power.
 * 
 * INSTRUCTIONS:
 * 1. Select "RoboBoard X3" in Arduino IDE
 * 2. Compile and upload sketch
 * 3. Connect Totem App and control the robot as usual
 */
// Read "invert motor" and "automatic braking" settings from Totem App
int Board_getDCInvert();
int Board_getDCAutobrake();
// Spin DC motor using Totem App settings
bool spinMotor(int port, int value) {
    if (value == 0) { DC[port].brake(Board_getDCAutobrake() ? 100 : 0); }
    else { DC[port].spin(Board_getDCInvert() ? -value : value); }
    return false;
}
// Override function called on app button click
bool appOverride(int cmd, int value) {
  // Redirect APP "/0/dc/power" command to Motor API
  if (cmd == TotemApp.cmdPowerA) { return spinMotor(0, value); }
  if (cmd == TotemApp.cmdPowerB) { return spinMotor(1, value); }
  if (cmd == TotemApp.cmdPowerC) { return spinMotor(2, value); }
  if (cmd == TotemApp.cmdPowerD) { return spinMotor(3, value); }
  return true;
}
void initRoboBoard() {
  // Display battery and connection state
  Board.setStatusRGB(true);
  // Enter charging mode when USB is plugged in
  Board.setChargingMode(true);
}
void setup() {
  // Set motor acceleration to limit instant current surge
  DC.setAccelerationTime(200);
  // Limit maximum motor power to 70% for less current usage
  DC.setRange(0, 70);
  // Register Totem App override function
  TotemApp.addOverride(appOverride);
  // Enable Totem App connectivity
  TotemApp.begin();
}
// Loop program
void loop() {
  // Empty
}