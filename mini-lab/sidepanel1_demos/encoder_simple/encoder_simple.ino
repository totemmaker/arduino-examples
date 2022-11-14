// Example of #1 I/O side panel Rotary Encoder
// Connect Arduino pins:
#define ENC_PIN_BTN 2
#define ENC_PIN_A 3
#define ENC_PIN_B 4

bool changed = false;
bool buttonPress = false;
int position = 0;

// Called when button is pressed
void btn_interrupt() {
  changed = true;
  // Button is pressed if pin state is LOW (GND)
  buttonPress = digitalRead(ENC_PIN_BTN) == LOW;
}
// Called when knob is rotated
void enc_interrupt() {
  changed = true;
  // Blink LED on encoder turn
  digitalWrite(13, !digitalRead(13));
  // Read turn direction
  if (digitalRead(ENC_PIN_B) == HIGH) {
    position--;
  }
  else {
    position++;
  }
}

void setup() {
  Serial.begin(115200);
  // Configure encoder pins as inputs
  pinMode(13, OUTPUT);
  pinMode(ENC_PIN_A, INPUT);
  pinMode(ENC_PIN_B, INPUT);
  pinMode(ENC_PIN_BTN, INPUT);
  // Register interrupt for ENC_PIN_A and ENC_PIN_BTN change
  // Only pins 2 and 3 can be used as interrupts in Atmega328p
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_BTN), btn_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), enc_interrupt, FALLING);
}

void loop() {
  if (changed) {
    // Print changed encoder state
    int button_state = digitalRead(ENC_PIN_BTN) == LOW;
    Serial.print("Button: ");
    Serial.print(button_state);
    Serial.print(". Position: ");
    Serial.println(position);
    changed = false;
  }
}
