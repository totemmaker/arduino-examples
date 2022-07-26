//Schmitt trigger demontstration project

//D15 == A1
#define OUTPUT_PIN  15
#define INPUT_PIN   A0

//To disable SCHMITT_TRIGGER, comment or delete this line
//#define SCHMITT_ON

#define THRESHOLD   512
#define HYSTERESIS  50

void setOutputState(bool newState){
  static bool state;

  if(state == newState){
    return;
  }

  state = newState;
  digitalWrite(OUTPUT_PIN, state);
  
  Serial.print(state);
  Serial.print(", THRESHOLD=");
  Serial.print(THRESHOLD);
  if(newState){
    Serial.println(" Turning output ON");
  }else{
    Serial.println(" Turning output OFF");
  }
}

void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("Schmitt trigger demo");
  Serial.print("Schmitt trigger: ");
#ifdef SCHMITT_ON
  Serial.println("ENABLED");
#else
  Serial.println("DISABLED");
#endif 
}

void loop() {

  int value = analogRead(INPUT_PIN);
  
#ifdef SCHMITT_ON
  if(value > THRESHOLD + HYSTERESIS){
    setOutputState(true);
  }

  if(value < THRESHOLD - HYSTERESIS){
    setOutputState(false);
  }

#else
  if(value >= THRESHOLD){
    setOutputState(true);
    
  }else{
    setOutputState(false);
  }
#endif
}
