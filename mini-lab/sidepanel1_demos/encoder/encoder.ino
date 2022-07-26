//Encoder example
#include <ClickEncoder.h>
#include <TimerOne.h>

#define ENC_A_PIN    9
#define ENC_B_PIN    10
#define ENC_BUTT_PIN 8

ClickEncoder encoder(ENC_A_PIN, ENC_B_PIN, ENC_BUTT_PIN, 4, LOW);
int encoderCounter;

static void encTimerISR(void){
  encoder.service();
}
void setup() {
  // put your setup code here, to run once:
  Timer1.initialize(1000);
  Timer1.attachInterrupt(encTimerISR);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  encoderCounter += encoder.getValue();
  Serial.print("Encoder: ");
  Serial.print(encoderCounter);
  Serial.print(", button: ");
  if(encoder.getButton() == ClickEncoder::Open){
    Serial.print("false");
  }else{
    Serial.print("true");
  }
  Serial.print("\n");
  
  delay(100);
  
}
