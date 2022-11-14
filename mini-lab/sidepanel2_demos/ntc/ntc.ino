// Example of #2 Sensor side panel NTC sensor
// Connect Arduino pins:
#define NTC_PIN A0

int R5 = 10000; // R5 resistor 10kOhm

// Get NTC resistance
// R      - resistor value of voltage divider
// adcMax - ADC resolution (1024 in 10b ADC)
// adc    - measured ADC value
// result - NTC resistance
long getNTCResistance(long R, long adcMax, long adc) {
  // Voltage divider equation
  return (adc*R)/((adcMax-1)-adc);
}

// Get NTC thermistor temperature Celsius
// R      - thermistor resistance
// result - temperature in degrees Celsius
float getNTCTemperature(float R) {
  // Coeficients of 10kOhm/25C NTC thermistor
  const float A = 2.108508173E-03;
  const float B = 0.7979204727E-04;
  const float C = 6.535076315E-07;
  // Steinhart & Hart equation
  float temp = 1 / (A + B * logf(R) + C * powf(logf(R), 3));
  temp -= 273.15; // To Kelvin to Celsius
  return temp;
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Get NTC analog value
  int ntc_adc = analogRead(A0);
  // Get NTC resistance
  long ntc_resistance = getNTCResistance(R5, (1<<10), ntc_adc);
  // Get NTC temperature
  float temp = getNTCTemperature(ntc_resistance);
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("C, Resistance: ");
  Serial.print(ntc_resistance);
  Serial.println("Ohm");
  delay(100);
}
