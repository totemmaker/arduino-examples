/* DESCRIPTION:
 * Use PS3 gamepad to drive Totem RoboCar https://totemmaker.net/product/robocar-chassis/
 * 
 * PREPARATIONS:
 * Follow instructions in https://docs.totemmaker.net/tutorials/04.SetupController/ to install required
 * libraries and setup controller.
 * 
 * INSTRUCTIONS:
 * 1. Select "RoboBoard X4" in Arduino IDE
 * 2. Compile and upload sketch
 * 3. Press PS (middle) button on remote controller
 * 4. Wait for controller LED to stop blinking and motors to beep
 * 
 * CONTROLS:
 * - Left stick up/down - drive forward/backward (throttle)
 * - Right stick left/right - steer left/right
 * - Circle button - beep
 * - Left trigger - brake
 * - Right trigger - throttle
 * - D-Pad left - steer 100% left
 * - D-Pad right - steer 100% right
 * - D-Pad up - shift gear forward (for right trigger throttle)
 * - D-Pad down - shift gear backward (for right trigger throttle)
 * - Select - Enable/Disable automatic braking
 * - Start  - Switch between slow and fast decay motor control modes
 *            (speed control or torque control)
 * 
 * RGB lights:
 * - Totem colors - driving / idle
 * - Red - braking
 * - White - driving backwards
 * - White / red - driving backwards and braking
 * 
 * LED controller:
 * - 1 - Slow decay mode selected (change with START)
 * - 2 - Fast decay mode selected (change with START)
 * - 4 - AutoBrake enabled (change with SELECT)
 */
#include <Arduino.h>

#include <Ps3Controller.h> // https://github.com/jvpernis/esp32-ps3
// Drive direction when throttle is pressed
enum {
  FORWARD = 1,
  BACKWARD = -1,
};
int gearboxDirection = FORWARD;
// LED lighting states
enum LedState {
  LedIdle,
  LedBrake,
  LedReverse,
  LedReverseBrake
};
// Change LED color of X4 and controller
void setLed(LedState state) {
  static LedState lastLed = LedIdle;
  // Update LED state only if state has changed
  if (state == lastLed) return;
  switch (state) {
    case LedIdle:
      RGB.colorTotem(); // Set to Totem colors
      break;
    case LedBrake:
      RGB.color(Color::Red); // Set to red
      break;
    case LedReverse:
      RGB.color(Color::White); // Set to white
      break;
    case LedReverseBrake:
      RGB.A.color(Color::Red); // Set A to white
      RGB.B.color(Color::White); // Set B to red
      RGB.C.color(Color::White); // Set C to red
      RGB.D.color(Color::Red); // Set D to white
      break;
  }
  lastLed = state;
}
// Change LED color depending on situation
void updateLed() {
  // Get speed (and direction) robot is driving at
  if (Drivetrain.getDrive() >= 0) {
    // Show brake lights if car is braking
    setLed(Drivetrain.getBrake() ? LedBrake : LedIdle);
  }
  else {
    // Show while lights if car is reversing
    setLed(Drivetrain.getBrake() ? LedReverseBrake : LedReverse);
  }
  // Controller LED
  static int lastPlayer = 0;
  int player = 0;
  player += DC.A.getFastDecay() ? 2 : 1; // LED 1 or 2 for decay mode
  player += DC.A.getAutobrake() ? 4 : 0; // LED 4 on/off for autobrake
  if (lastPlayer != player) { // call "setPlayer" only if value has changed
    Ps3.setPlayer(player);
    lastPlayer = player;
  }
  //           led4  led3  led2  led1
  // player 1                    1
  // player 2              1
  // player 3        1
  // player 4  1
  // player 5  1                 1
  // player 6  1           1
  // player 7  1     1
  // player 8  1     1           1
  // player 9  1     1     1
  // player 10 1     1     1     1
}
// Events received from controller
void onControllerEvent() {
  // D-Pad UP is pressed
  if (Ps3.event.button_down.up) gearboxDirection = FORWARD;
  // D-Pad DOWN is pressed
  if (Ps3.event.button_down.down) gearboxDirection = BACKWARD;
  // SELECT button is pressed
  if (Ps3.event.button_down.select) {
    // Enable or disable auto braking
    DC.setAutobrake(DC.A.getAutobrake() ? 0 : 100);
  }
  // START button is pressed
  if (Ps3.event.button_down.start) {
    // Switch DC driver between slow and fast decay modes
    DC.A.getFastDecay() ? DC.setSlowDecay() : DC.setFastDecay();
    DC.setFrequency(DC.A.getFastDecay() ? 50 : 20000);
  }
}
// Called before "setup()". For overriding initial RoboBoard settings
void initRoboBoard() {
  Board.setStatusSound(true); // Beep on power on
}

void setup() {
  /////////////////////////////////
  // Configure Drivetrain module
  /////////////////////////////////
  Drivetrain.setWheelLeft(DC.A);
  Drivetrain.setWheelRight(DC.B);
  Drivetrain.setDriveSteer();
  // Invert servo spin direction
  Servo.A.setInvert(true);
  // Map servo A positions to full left: -38, center: -7, full right: 18. Translates to [-100:0:100]%
  // More info: https://docs.totemmaker.net/roboboard/api/servo/#calibration-trimming
  Servo.A.setTrim(-38, -7, 18);
  /////////////////////////////////
  // Setup PS controller library
  /////////////////////////////////
  // Attach controller events handler
  Ps3.attach(onControllerEvent);
  // Match MAC address stored in controller
  Ps3.begin((char*)"00:02:03:04:05:06");
  // Wait for controller to connect
  while(!Ps3.isConnected()) { delay(1); }
  // Beep on controller connection
  DC.tone(4000, 200); // Play 4kHz for 200ms
  delay(300);
  DC.tone(4000, 200);
  delay(300);
}

void loop() {
  // Restart board if controller has disconnected
  if (!Ps3.isConnected()) { Board.restart(); }
  /////////////////////////////////
  // Read state of controller buttons and axis
  /////////////////////////////////
  // Joystick module helps to convert raw controller values to [-100:100]% percentage.
  // Also applies quadratic function to joysticks for more precise control.
  int steer = Joystick::getAxis(3, Ps3.data.analog.stick.rx); // Read right stick X axis position (steer)
  int brake = Joystick::getTrigger(Ps3.data.analog.button.l2); // Left shoulder (brake)
  int drive = Joystick::getTrigger(Ps3.data.analog.button.r2); // Right shoulder (throttle)
  drive = drive * gearboxDirection; // Apply 
  if (drive == 0) { // Override joystick if throttle is pressed
    drive = Joystick::getAxis(-Ps3.data.analog.stick.ly); // Read left strick Y axis position (throttle/reverse)
  }
  // Steer to maximum angle if D-Pad keys are pressed
  if (Ps3.data.button.left) steer = -100;
  if (Ps3.data.button.right) steer = 100;
  /////////////////////////////////
  // Update Drivetrain with new drive speed and steer values
  /////////////////////////////////
  if (Ps3.data.button.circle && drive == 0) {
    DC.tone(4000); // Output 4kHz tone on DC motors
  }
  else {
    Drivetrain.handbrake(brake);
    Drivetrain.driveTurn(drive, steer);
  }
  /////////////////////////////////
  // Update RoboBoard X4 RGB lights and controller LED
  /////////////////////////////////
  updateLed();
}
