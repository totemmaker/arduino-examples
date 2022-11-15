// Example of #1 I/O side panel button press detect
// Takes care of pin pull-up and debouncing
// Connect Arduino pins:
int button_pin[3] = {5, 6, 7};
// Hold button state (pressed or not)
bool button[3];

void setup() {
  // Initialize pins
  pinMode(button_pin[0], INPUT_PULLUP);
  pinMode(button_pin[1], INPUT_PULLUP);
  pinMode(button_pin[2], INPUT_PULLUP);
  // Initialize serial
  Serial.begin(115200);
}

bool check_buttons() {
  static long press_time[3];
  bool changed = false;
  for (int i=0; i<3; i++) {
    // Remember button press time
    if (digitalRead(button_pin[i]) == LOW) {
      if (press_time[i] == 0) press_time[i] = millis();
    }
    else press_time[i] = 0;
    // Check if button is pressed for longer than 10ms (debounce check)
    int state = (press_time[i] != 0) && (millis() - press_time[i]) > 10;
    // Update button state
    if (state != button[i]) {
      button[i] = state;
      changed = true;
    }
  }
  // Return true if any button state was changed
  return changed;
}

void loop() {
  // Run function to check buttons state
  if (check_buttons()) {
    // Print buttons state if changed
    Serial.print("Button A: ");
    Serial.print(button[0]);
    Serial.print(", B: ");
    Serial.print(button[1]);
    Serial.print(", C: ");
    Serial.println(button[2]);
  }
}
