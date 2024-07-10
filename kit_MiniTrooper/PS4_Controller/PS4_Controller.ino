/* DESCRIPTION:
 * Use PS4 gamepad to drive Totem Mini Trooper https://totemmaker.net/product/mini-trooper/
 * 
 * PREPARATIONS:
 * Follow instructions in https://docs.totemmaker.net/tutorials/04.SetupController/ to install required
 * libraries and setup controller.
 * 
 * INSTRUCTIONS:
 * 1. Select "RoboBoard X3" in Arduino IDE
 * 2. Compile and upload sketch
 * 3. Press PS (middle) button on remote controller
 * 4. Wait for controller LED to stop blinking and motors to beep
 * 
 * !!! IF FLIPPER DOES NOT MOVE PROPERLY, SEE LINE 69 !!!
 * 
 * REMOTE USAGE:
 * L,R triggers - move flipper
 * L,R shoulders - move flipper slowly
 * Triangle button - open flipper
 * Cross button - close flipper
 * Circle button - beep
 * D-Pad - drive robot
 * Left stick up/down - drive forward/backward
 * Right stick left/right - drive left/right
 * - Share   - Enable/Disable automatic braking
 * - Options - Switch between slow and fast decay motor control modes
 *             (speed control or torque control)
 */
#include <Arduino.h>

#include <PS4Controller.h> // https://github.com/aed3/PS4-esp32

// Events received from controller
void onControllerEvent() {
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
  Board.setChargingMode(true); // Charge mode when USB is plugged in
  Board.setStatusSound(true); // Beep on power on
  Board.setStatusRGB(true); // Display battery status on RGB
}

void setup() {
  /////////////////////////////////
  // Configure Drivetrain module
  /////////////////////////////////
  Drivetrain.setWheelLeft(DC.A, false);
  Drivetrain.setWheelRight(DC.B, false);
  Drivetrain.setDriveTank();
  // Set 500ms DC motors acceleration for better grip
  DC.setAccelerationTime(500);
  /////////////////////////////////
  // !! IF FLIPPER DOES NOT MOVE !!
  /////////////////////////////////
  // Some kits contain servo motor that works in slightly different range.
  // This results in motor stop moving if setting out of range position.
  // In that case, uncomment this line, to change motor parameters:
//  Servo.A.setMotor(180, 650, 2350);
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

class Flipper {
  const int angleClose = 0;
  const int angleOpen = 180;
  const int slowMoveRPM = 15;
  // If USB is plugged in, prevent flipper from closing on it
  int getAngleClose() { return Board.isUSB() ? 60 : angleClose; }
public:
  // Move to 180 degree angle at 10 RPM
  void moveUpSlow() { Servo.A.spinAngleRPM(angleOpen, slowMoveRPM); }
  // Move to 0 degree angle at 10 RPM
  void moveDownSlow() { Servo.A.spinAngleRPM(getAngleClose(), slowMoveRPM); }
  // Stop at current angle (if moving with set RPM)
  void moveStop() { Servo.A.spinPulse(Servo.A.getPulse()); }
  // Open fully at max speed
  void open() { Servo.A.spinAngle(angleOpen); }
  // Close fully at max speed
  void close() { Servo.A.spinAngle(getAngleClose()); }
  // Open at specific position [0:100]%
  void openPos(int pos) { Servo.A.spinAngle(map(pos, 0, 100, getAngleClose(), angleOpen)); }
} flipper;

void loop() {
  // Restart board if controller has disconnected
  if (!PS4.isConnected()) { Board.restart(); }
  /////////////////////////////////
  // Read state of controller buttons and axis
  /////////////////////////////////
  // Joystick module helps to convert raw controller values to [-100:100]% percentage.
  // Also applies quadratic function to joysticks for more precise control.
  static int lastFlipPos = -1;
  int flipPos = Joystick::getTrigger(max(PS4.data.analog.button.l2, PS4.data.analog.button.r2));
  int drive = Joystick::getAxis(3, PS4.data.analog.stick.ly);
  int turn = Joystick::getAxis(3, PS4.data.analog.stick.rx);
  /////////////////////////////////
  // Move flipper
  /////////////////////////////////
  // Control using (L2), (R2) triggers (only if changed)
  if (flipPos != lastFlipPos) { flipper.openPos(flipPos); lastFlipPos = flipPos; }
  // Right shoulder pressed (R1)
  else if (PS4.data.button.r1) flipper.moveUpSlow();
  // Left shoulder pressed (L1)
  else if (PS4.data.button.l1) flipper.moveDownSlow();
  // Triangle button pressed
  else if (PS4.data.button.triangle) flipper.open();
  // Cross button pressed
  else if (PS4.data.button.cross) flipper.close();
  // Stop flipper at last position
  else { flipper.moveStop(); }
  /////////////////////////////////
  // Update Drivetrain with new drive speed and steer values
  /////////////////////////////////
  if (PS4.data.button.left)  turn -= 100;
  if (PS4.data.button.right) turn += 100;
  if (PS4.data.button.up)    drive += 100;
  if (PS4.data.button.down)  drive -= 100;
  if (PS4.data.button.circle) {
    DC.tone(4000); // Play 4kHz tone
  }
  else {
    Drivetrain.driveTurn(drive, turn); // Drive robot
  }
}
