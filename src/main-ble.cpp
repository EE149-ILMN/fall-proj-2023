#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>
#include <BLE2902.h>
#include <Arduino.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool twitched = false;

#define SERVICE_UUID        "deadbeef-beef-beef-beef-deadbeefbeef"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


const int x_pin = 13;
const int y_pin = 14;
const int z_pin = 15;

float x_raw, y_raw, z_raw;
float x_sen, y_sen, z_sen;
float x_bias, y_bias, z_bias;
float x_accel, y_accel, z_accel;

void read_accel();
void scale_accel();
void print_accel();

void read_accel() {
  x_raw = (float) analogRead(x_pin) / (float)1023;
  y_raw = (float) analogRead(y_pin) / (float)1023;
  z_raw = (float) analogRead(z_pin) / (float)1023; 
}

void scale_accel() {
  x_accel = (x_raw - x_bias) / x_sen;
  y_accel = (y_raw - y_bias) / y_sen;
  z_accel = (z_raw - z_bias) / z_sen;
}
void print_accel(){
  Serial.printf("x: %f y: %f z: %f \n", x_accel, y_accel, z_accel);
}

class ConnectionCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("CONNECTED!");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("DISCONNECTED!");
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  
  /* Setup the accelerometer */
  
  x_sen = 0.3;
  y_sen = 0.3;
  z_sen = 0.3;

  x_bias = 1.5;
  y_bias = 1.5;
  z_bias = 1.5;

  BLEDevice::init("TwitchDetect");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ConnectionCallback());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic->setValue("TwitchDetect Init");
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    int value = 1;

    if (deviceConnected /*&& twitched*/) {
        Serial.printf("Sending value: %d\n", 1);
        pCharacteristic->setValue((uint8_t*)&value, 4);
        // pCharacteristic->setValue(1);
        pCharacteristic->notify();
        // value++;
        delay(3);
    }

    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    read_accel();
    scale_accel();
    //print_accel();
    delay(100);
}