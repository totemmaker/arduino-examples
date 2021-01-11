//Alarm system activated by sound. Uses buzzer and microphone with integrated
//amplifier modules from sensor side panel.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MICPIN  A0
//D15 == A1 pin
#define ALARMPIN 15

//anything above trigger value will trigger alarm
#define TRIGGERVALUE  100
//alarm will ring for this amount of mili seconds
#define ALARMTIMEOUT  5000

void soundAlarm(){

  int elapsed = 0;
  while(elapsed < ALARMTIMEOUT){
    digitalWrite(ALARMPIN, 1);
    delay(250);
    elapsed += 250;
    digitalWrite(ALARMPIN, 0);
    delay(250);
    elapsed += 250;
  }
}

int updateFilterValue(int *avg, int value){

  *avg = *avg * 0.8 + value * 0.2;
  return value - *avg;
}

int avg = 0;
int alarmThreshold = 0;

void setup() {
  //Remove these if you don't want to use display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);
  display.println("SoundAlarm");
  display.display();
  delay(1000);
  display.clearDisplay();

  //Remove end

  Serial.begin(115200);
  Serial.println("Hello");
  
  pinMode(ALARMPIN, OUTPUT);
  
  //collect a number of samples to get reference baseline of sound
  for(int i=0; i<512; i++){
    updateFilterValue(avg, analogRead(MICPIN));
  }
  
}

void updateScreen(int value){
  static int values[64];
  static int start_idx;

  int currValue = value / 16;
  if(currValue > display.height()-1){
    currValue = display.height() - 1;
  }
  
  values[start_idx] = currValue;

  display.clearDisplay();
  for(int i=0;i<64;i++){
    display.fillRect(i*2, display.height()-1-values[(start_idx+i)%64], 2, values[(start_idx+i)%64], SSD1306_WHITE);
  }
  start_idx += 1;
  start_idx %= 64;
  display.display();
}

void loop() {
  //this will let us know how far away we're from the moving average
  int rawValue = analogRead(MICPIN);
  int value = abs(updateFilterValue(&avg, rawValue));
  updateScreen(rawValue);
  delay(100);

  //only react to sounds that are long and powerful enough:
  if(value >= TRIGGERVALUE){
    alarmThreshold += 1;

    //if sound has went long enough sound the alarm:
    if(alarmThreshold >= 3){
      soundAlarm();
      alarmThreshold = 0;
    }
  }else{
    //Reset in cases where the sound was too short
    alarmThreshold = 0;
  }
}
