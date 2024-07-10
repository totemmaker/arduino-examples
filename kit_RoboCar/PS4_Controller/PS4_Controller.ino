/* DESCRIPTION:
 * Use PS4 gamepad to drive Totem RoboCar https://totemmaker.net/product/robocar-chassis/
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
 * - Share   - Enable/Disable automatic braking
 * - Options - Switch between slow and fast decay motor control modes
 *             (speed control or torque control)
 * 
 * RGB lights:
 * - Totem colors - driving / idle
 * - Red - braking
 * - White - driving backwards
 * - White / red - driving backwards and braking
 * 
 * LED controller:
 * - Blinking - connecting to RoboCar
 * - Green - connected
 * - White - driving backwards
 * - Red - braking
 */
#include <Arduino.h>
#include <Wire.h>

#include <PS4Controller.h> // https://github.com/aed3/PS4-esp32
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
      PS4.setLed(0, 255, 0); // Set green
      PS4.sendToController();
      break;
    case LedBrake:
      RGB.color(Color::Red); // Set to red
      PS4.setLed(255, 0, 0); // Set red
      PS4.sendToController();
      break;
    case LedReverse:
      RGB.color(Color::White); // Set to white
      PS4.setLed(255, 255, 255); // Set white
      PS4.sendToController();
      break;
    case LedReverseBrake:
      RGB.A.color(Color::Red); // Set A to white
      RGB.B.color(Color::White); // Set B to red
      RGB.C.color(Color::White); // Set C to red
      RGB.D.color(Color::Red); // Set D to white
      PS4.setLed(255, 255, 255); // Set white
      PS4.sendToController();
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
}
// Events received from controller
void onControllerEvent() {
  // D-Pad UP is pressed
  if (PS4.event.button_down.up) gearboxDirection = FORWARD;
  // D-Pad DOWN is pressed
  if (PS4.event.button_down.down) gearboxDirection = BACKWARD;
  // SHARE button is pressed
  if (PS4.event.button_down.share) {
    // Enable or disable auto braking
    DC.setAutobrake(DC.A.getAutobrake() ? 0 : 100);
  }
  // OPTIONS button is pressed
  if (PS4.event.button_down.options) {
    // Switch DC driver between slow and fast decay modes
    DC.A.getFastDecay() ? DC.setSlowDecay() : DC.setFastDecay();
    DC.setFrequency(DC.A.getFastDecay() ? 50 : 20000);
  }
}
// Called before "setup()". For overriding initial RoboBoard settings
void initRoboBoard() {
  Board.setStatusSound(true); // Beep on power
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
  // Setup I2C communication to read IMU
  /////////////////////////////////
  Wire.begin(SDA, SCL, 400000);
  /////////////////////////////////
  // Setup PS controller library
  /////////////////////////////////
  // Attach controller events handler
  PS4.attach(onControllerEvent);
  // Match MAC address stored in controller
  PS4.begin((char*)"00:02:03:04:05:06");
  // Wait for controller to connect
  while(!PS4.isConnected()) { delay(1); }
  // Set controller color to green
  PS4.setLed(0, 255, 0);
  PS4.sendToController();
  // Beep on controller connection
  DC.tone(4000, 200); // Play 4kHz for 200ms
  delay(300);
  DC.tone(4000, 200);
  delay(300);
}

void loop() {
  // Restart board if controller has disconnected
  if (!PS4.isConnected()) { Board.restart(); }
  /////////////////////////////////
  // Read state of controller buttons and axis
  /////////////////////////////////
  // Joystick module helps to convert raw controller values to [-100:100]% percentage.
  // Also applies quadratic function to joysticks for more precise control.
  int steer = Joystick::getAxis(3, PS4.data.analog.stick.rx); // Read right stick X axis position (steer)
  int brake = Joystick::getTrigger(PS4.data.analog.button.l2); // Left shoulder (brake)
  int drive = Joystick::getTrigger(PS4.data.analog.button.r2); // Right shoulder (throttle)
  drive = drive * gearboxDirection; // Apply 
  if (drive == 0) { // Override joystick if throttle is pressed
    drive = Joystick::getAxis(PS4.data.analog.stick.ly); // Read left strick Y axis position (throttle/reverse)
  }
  // Steer to maximum angle if D-Pad keys are pressed
  if (PS4.data.button.left) steer = -100;
  if (PS4.data.button.right) steer = 100;
  /////////////////////////////////
  // Update Drivetrain with new drive speed and steer values
  /////////////////////////////////
  if (PS4.data.button.circle && drive == 0) {
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
  /////////////////////////////////
  // Read IMU to detect force over 4G
  /////////////////////////////////
  auto imu = IMU.read();
  static int rumbleEnd = 0;
  if (imu.getX_G() > 4 || imu.getY_G() > 4 || imu.getZ_G() > 4) {
    PS4.setRumble(255, 255); // Spin both motors at max
    PS4.sendToController();
    rumbleEnd = millis() + 200; // Rumble for 200ms
  }
  if (rumbleEnd && rumbleEnd < millis()) {
    // Stop rumble if running
    PS4.setRumble(0, 0);
    PS4.sendToController();
    rumbleEnd = 0;
  }
}
