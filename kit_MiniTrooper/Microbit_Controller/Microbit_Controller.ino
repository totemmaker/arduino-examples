/* DESCRIPTION:
 * Use BBC Micro:bit to drive Totem Mini Trooper https://totemmaker.net/product/mini-trooper/
 * 
 * PREPARATIONS:
 * Follow instructions in https://docs.totemmaker.net/remote-control/microbit/ to setup Micro:bit
 * 
 * INSTRUCTIONS:
 * 1. Power on Micro:bit
 * 2. Select "RoboBoard X3" in Arduino IDE
 * 3. Compile and upload sketch
 * 4. Use Micro:bit buttons to control
 * 
 * REMOTE USAGE:
 * Micro:bit button A - turn left
 * Micro:bit button B - turn right
 * Micro:bit button A + B - drive forward
 */
#include <Arduino.h>

// Micro:bit communication initialization
void microbitBegin();
// Micro:bit [bluetooh on data received] (send to micro:bit)
void microbitWriteString(String data);

// Micro:bit [bluetooth uart write] (receive from micro:bit)
void microbitOnWrite(String data) {
  int number = data.toInt();
  int buttonA = number & 1;
  int buttonB = number & 2;
  if (buttonA && buttonB) Drivetrain.driveTurn(100, 0);
  else if (buttonA) Drivetrain.driveTurn(0, -100);
  else if (buttonB) Drivetrain.driveTurn(0, 100);
  else Drivetrain.brake();
  // Serial.println(data);
}

void onButtonChange() {
  // Send (BOOT) button state to Micro:bit
  microbitWriteString(Button.isPressed() ? "1," : "0,"); // ',' is used as delimiter
}

// Called before "setup()". For overriding initial RoboBoard settings
void initRoboBoard() {
  // Board.setChargingMode(true); // Charge mode when USB is plugged in
  Board.setStatusSound(true); // Beep on power on and connection
  Board.setStatusRGB(true); // Display battery status on RGB
}
// Begin program
void setup() {
  Serial.begin(115200);
  /////////////////////////////////
  // Configure Drivetrain module
  /////////////////////////////////
  Drivetrain.setWheelLeft(DC.A, false);
  Drivetrain.setWheelRight(DC.B, false);
  Drivetrain.setDriveTank();
  Drivetrain.setMaxSpeed(35); // Maximum drive speed %
  // Find and connect Micro:bit
  microbitBegin();
  // Micro:bit connected
  Serial.println("Connected to BBC Micro:bit");
  // Add (BOOT) button press/release event
  Button.addEvent(onButtonChange);
}
// Loop program
void loop() {

}

/**
 * Bluetooth code required to establish connection and exchange data with Micro:bit
 */
#include "BLEDevice.h"
namespace BT {
// Remote services UUID
static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID  charRxUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID  charTxUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
static BLERemoteCharacteristic* pRemoteCharacteristicTx;
static struct {
  BLEAddress address;
  esp_ble_addr_type_t type;
} discoveredDevice = {BLEAddress(""),BLE_ADDR_TYPE_PUBLIC};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // No proper way to detect "number" or "string" block. Always return string
  microbitOnWrite(String(pData, length));  
}

static class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) { }
  void onDisconnect(BLEClient* pclient) {
    Board.restart(); // Restart RoboBoard if connection is lost
  }
} myClientCallbacks;

static class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // Scan result received
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Check if result contains microbit name
    if (advertisedDevice.haveName()) {
      if (advertisedDevice.getName().compare(0, 13, "BBC micro:bit") == 0) {
        // Store connection data
        BLEDevice::getScan()->stop();
        discoveredDevice.address = advertisedDevice.getAddress();
        discoveredDevice.type = advertisedDevice.getAddressType();
      }
    }
  }
} myAdvertisedDeviceCallbacks;

} // namespace BT

void microbitBegin() {
  // Initialize BLE environment
  BLEDevice::init("");
  // Find Micro:bit
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(&BT::myAdvertisedDeviceCallbacks);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, false);
  // Establish connection to Micro:bit
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(&BT::myClientCallbacks);
  if (!pClient->connect(BT::discoveredDevice.address, BT::discoveredDevice.type)) {
    Board.restart(); // Restart RoboBoard if connection failed
    return;
  }
  // Obtain a references to service and characteristics
  BLERemoteService* pRemoteService = pClient->getService(BT::serviceUUID);
  BLERemoteCharacteristic* pRemoteCharacteristicRx = pRemoteService->getCharacteristic(BT::charRxUUID);
  BT::pRemoteCharacteristicTx = pRemoteService->getCharacteristic(BT::charTxUUID);
  // Register indication
  pRemoteCharacteristicRx->registerForNotify(BT::notifyCallback, false, true);
}

void microbitWriteString(String data) {
  // Send data to TX characteristic
  BT::pRemoteCharacteristicTx->writeValue((uint8_t*)data.c_str(), data.length());
}
