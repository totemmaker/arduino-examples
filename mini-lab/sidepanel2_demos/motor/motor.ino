//PWM-based motor speed and direction control

//D14 == A0 pin
#define MOTORFW_PIN 10
//D15 == A1 pin
#define MOTORRW_PIN 11

//limit maximum motor speed PWM value
#define MAXSPEED  100

//sign of speed controls the direction of motor
void setMotorSpeed(int speed){

  //clamp speed to maximum allowed value range
  if(speed > MAXSPEED){
    speed = MAXSPEED;
  }
  if(speed < -MAXSPEED){
    speed = -MAXSPEED;
  }

  //depending on direction route signal to FW or RW pin
  if(speed >= 0){
    analogWrite(MOTORFW_PIN, speed);
    analogWrite(MOTORRW_PIN, 0);
  }else{
    analogWrite(MOTORFW_PIN, 0);
    analogWrite(MOTORRW_PIN, speed);
  }
}

int speedValue = 0;
int delta = 2;

void setup() {

  //setup motor control pins as outputs
  pinMode(MOTORFW_PIN, OUTPUT);
  pinMode(MOTORRW_PIN, OUTPUT);

  //start with idle state
  setMotorSpeed(speedValue);
}

void loop() {

  //soft start rotation from one direction to another

  //if we reached our limits, invert direction of speed change
  if(speedValue >= MAXSPEED || speedValue <= -MAXSPEED){
     delta *= -1;
  }

  //apply speed change and wait for a bit
  speedValue += delta;
  setMotorSpeed(speedValue);
  delay(100);
}
