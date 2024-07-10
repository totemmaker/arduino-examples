/* DESCRIPTION:
 * Use PS3 gamepad to drive Totem Gripper Bot https://totemmaker.net/product/gripper-bot-smartphone-app-controlled-car/
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
 * REMOTE USAGE:
 * Left trigger - open gripper
 * Right trigger - close gripper
 * Left shoulder - retract arm
 * right shoulder - extend arm
 * Triangle button - fully extend arm
 * Cross button - fully retract arm
 * Square button - hold to slowly retract / extend arm
 * Circle button - beep
 * D-Pad - drive robot
 * Left stick up/down - drive forward/backward
 * Right stick left/right - drive left/right
 * - SELECT - Enable/Disable automatic braking
 * - START  - Switch between slow and fast decay motor control modes
 *            (speed control or torque control)
 * 
 * LED controller:
 * - 1 - Slow decay mode selected (change with START)
 * - 2 - Fast decay mode selected (change with START)
 * - 4 - AutoBrake enabled (change with SELECT)
 */
#include <Arduino.h>

#include <Ps3Controller.h> // https://github.com/jvpernis/esp32-ps3

// Change controller LED depending on situation
void updateLed() {
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
class Gripper {
  const int angleClose = 55;
  const int angleOpen = 115;
  int position = 0; // 0 - closed, 100 - open
public:
  // Invert spin direction
  Gripper() { Servo.A.setInvert(true); }
  // Set absolute claws position [0:100]%
  void setPosition(int pos) {
    position = pos;
    Servo.A.spinAngle(map(pos, 0, 100, angleClose, angleOpen));
  }
  // Amount of % to open claws
  void open(int amount) {
    if (position < amount) {
      setPosition(amount);
    }
  }
  // Amount of % to close claws
  void close(int amount) {
    amount = 100-amount;
    if (position > amount) {
      setPosition(amount);
    }
  }
} gripper;

class Arm {
  const int angleRetract = 20;
  const int angleExtend = 120;
  const int speedRPM = 10;
public:
  // Invert spin direction
  Arm() { Servo.B.setInvert(true); }
  // Slowly extend arm. If "slow" - do it extra slow (1 RPM)
  void extend(bool slow) { Servo.B.spinAngleRPM(angleExtend, slow ? 1 : speedRPM); }
  // Slowly retract arm. If "slow" - do it extra slow (1 RPM)
  void retract(bool slow) { Servo.B.spinAngleRPM(angleRetract, slow ? 1 : speedRPM); }
  // Stop moving arm at current angle (if moving with set RPM)
  void stop() { Servo.B.spinPulse(Servo.B.getPulse()); }
  // Extend arm fully at max speed
  void extendFull() { Servo.B.spinAngle(angleExtend); }
  // Retract arm fully at max speed
  void retractFull() { Servo.B.spinAngle(angleRetract); }
  // Set absolute arm position [0:100]%
  void setPosition(int pos) {
    Servo.B.spinAngle(map(pos, 0, 100, angleRetract, angleExtend));
  }
} arm;

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
  Drivetrain.setWheelLeftFront(DC.C, false);
  Drivetrain.setWheelRightFront(DC.D, false);
  Drivetrain.setDriveTank();
  /////////////////////////////////
  // Setup PS controller library
  /////////////////////////////////
  // Attach controller events handler
  Ps3.attach(onControllerEvent);
  // Match MAC address stored in controller
  Ps3.begin((char*)"00:02:03:04:05:06");
  // Wait for controller to connect
  while(!Ps3.isConnected()) { delay(1); }
  // Move arm and gripper to start position
  arm.setPosition(10);
  gripper.setPosition(60);
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
  int gripClose = Joystick::getTrigger(Ps3.data.analog.button.r2); // Right trigger (R2)
  int gripOpen = Joystick::getTrigger(Ps3.data.analog.button.l2); // Left trigger (L2)
  int drive = Joystick::getAxis(3, -Ps3.data.analog.stick.ly); // Left joystick Y axis
  int turn = Joystick::getAxis(3, Ps3.data.analog.stick.rx); // Right joystick X axis
  /////////////////////////////////
  // Move gripper
  /////////////////////////////////
  gripper.close(gripClose);
  gripper.open(gripOpen);
  /////////////////////////////////
  // Move arm
  /////////////////////////////////
  // Right shoulder pressed (R1) (extract)
  if (Ps3.data.button.r1) arm.extend(Ps3.data.button.square);
  // Left shoulder pressed (L1) (retract)
  else if (Ps3.data.button.l1) arm.retract(Ps3.data.button.square);
  // Triangle button pressed (extract)
  else if (Ps3.data.button.triangle) arm.extendFull();
  // Cross button pressed (retract)
  else if (Ps3.data.button.cross) arm.retractFull();
  // Stop flipper at last position (idle)
  else { arm.stop(); }
  /////////////////////////////////
  // Update Drivetrain with new drive speed and steer values
  /////////////////////////////////
  if (Ps3.data.button.left)  turn -= 100;
  if (Ps3.data.button.right) turn += 100;
  if (Ps3.data.button.up)    drive += 100;
  if (Ps3.data.button.down)  drive -= 100;
  if (Ps3.data.button.circle) {
    DC.tone(4000); // Play 4kHz tone
  }
  else {
    turn = map(turn, 0, 100, 0, 70); // Limit maximum turn speed
    Drivetrain.driveTurn(drive, turn); // Drive robot
  }
  /////////////////////////////////
  // Update PS3 controller LED
  /////////////////////////////////
  updateLed();
}
