//Humidity and temperature sensor project
#include <DHT.h>
#include <Wire.h>

//Remove this if you don't want to use display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//Remove end


//D17 == A3 pin
#define DHTPIN  17
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  dht.begin();
  Serial.begin(115200);


  //Remove these if you don't want to use display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  //Remove end

}

void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if(isnan(h) || isnan(t)){
    Serial.println("Failed to read sensor");
    return;
  }

  Serial.print("Humidity:");
  Serial.print(h);
  Serial.print(", Temperature:");
  Serial.print(t);
  Serial.print("\n");

  //Remove these lines if you don't want to use display
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print("Temp:");
  display.println(t);
  display.print("Humid:");
  display.print(int(h));
  display.println("%");
  display.display();
  //Remove end
}
