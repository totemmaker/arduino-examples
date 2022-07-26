//RGB color mixing through PWM outputs

#define R_PIN   9
#define G_PIN   10
#define B_PIN   11
//place a limit to how bright LED can become
#define MAX_BRIGHTNESS 64

#define INPUT_PIN A0

void setColor(float r, float g, float b){

  //inputs are in a range of 0..1, and analogWrite expects 0..255
  //also we're inverting output to correspond to the way LED is connected
  analogWrite(R_PIN, 255 - MAX_BRIGHTNESS*r);
  analogWrite(G_PIN, 255 - MAX_BRIGHTNESS*g);
  analogWrite(B_PIN, 255 - MAX_BRIGHTNESS*b);
}

//these functions converts HSV color space to RGB
//this makes it easier for us to change colors according to hue
float fract(float x) { return x - int(x); }
float mix(float a, float b, float t) { return a + (b - a) * t; }
float step(float e, float x) { return x < e ? 0.0 : 1.0; }
float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

void setup() {
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  pinMode(INPUT_PIN, INPUT);

  //start with LED turned off
  setColor(0,0,0);
}

void loop() {

  //read out potentiometer position
  int value = analogRead(INPUT_PIN);
  //convert it into a hue in a range 0..1
  float hue = (float)value / 1024;
  
  //calculate RGB values from hue
  float rgb[3];
  hsv2rgb(hue, 1.0, 1.0, rgb);
  setColor(rgb[0], rgb[1], rgb[2]);
  delay(30);
}
