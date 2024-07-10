/* DESCRIPTION:
 * Use PS4 gamepad to drive Totem Gripper Bot https://totemmaker.net/product/gripper-bot-smartphone-app-controlled-car/
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
  PS4.attach(onControllerEvent);
  // Match MAC address stored in controller
  PS4.begin((char*)"00:02:03:04:05:06");
  // Wait for controller to connect
  while(!PS4.isConnected()) { delay(1); }
  // Set controller color to green
  PS4.setLed(0, 255, 0);
  PS4.sendToController();
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
  if (!PS4.isConnected()) { Board.restart(); }
  /////////////////////////////////
  // Read state of controller buttons and axis
  /////////////////////////////////
  // Joystick module helps to convert raw controller values to [-100:100]% percentage.
  // Also applies quadratic function to joysticks for more precise control.
  int gripClose = Joystick::getTrigger(PS4.data.analog.button.r2); // Right trigger (R2)
  int gripOpen = Joystick::getTrigger(PS4.data.analog.button.l2); // Left trigger (L2)
  int drive = Joystick::getAxis(3, PS4.data.analog.stick.ly); // Left joystick Y axis
  int turn = Joystick::getAxis(3, PS4.data.analog.stick.rx); // Right joystick X axis
  /////////////////////////////////
  // Move gripper
  /////////////////////////////////
  gripper.close(gripClose);
  gripper.open(gripOpen);
  /////////////////////////////////
  // Move arm
  /////////////////////////////////
  // Right shoulder pressed (R1) (extract)
  if (PS4.data.button.r1) arm.extend(PS4.data.button.square);
  // Left shoulder pressed (L1) (retract)
  else if (PS4.data.button.l1) arm.retract(PS4.data.button.square);
  // Triangle button pressed (extract)
  else if (PS4.data.button.triangle) arm.extendFull();
  // Cross button pressed (retract)
  else if (PS4.data.button.cross) arm.retractFull();
  // Stop flipper at last position (idle)
  else { arm.stop(); }
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
