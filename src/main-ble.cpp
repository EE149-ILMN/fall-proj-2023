#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>
#include <BLE2902.h>
#include <Arduino.h>

#include "tflite-micro/lite/micro/all_ops_resolver.h"
#include "tflite-micro/lite/micro/micro_error_reporter.h"
#include "tflite-micro/lite/micro/micro_interpreter.h"
#include "tflite-micro/lite/schema/schema_generated.h"
#include "tflite-micro/lite/version.h"

// Tensorflow
// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;
// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// BLuetooth
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool twitched = false;

#define SERVICE_UUID        "deadbeef-beef-beef-beef-deadbeefbeef"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Accel
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

void setup() {
  Serial.begin(115200);

  /* TF setup */
  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);
  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();
  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

  /* Bluetooth Setup */
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
  
  /* Setup the accelerometer */
  x_sen = 0.3;
  y_sen = 0.3;
  z_sen = 0.3;

  x_bias = 1.5;
  y_bias = 1.5;
  z_bias = 1.5;
}

void loop() {
    // Sample accel and run inference
    read_accel();
    scale_accel();
    tflInputTensor->data.f[0] = x_accel;
    tflInputTensor->data.f[1] = y_accel;
    tflInputTensor->data.f[2] = z_accel;
    TfLiteStatus invokeStatus = tflInterpreter->Invoke();
      if (invokeStatus != kTfLiteOk) {
        Serial.println("Invoke failed!");
        while (1);
        return;
      }
    int value = tflOutputTensor.f[0];
    
    if (deviceConnected) {
        Serial.printf("Sending value: %d\n", value);
        pCharacteristic->setValue((uint8_t*)&value, 4);
        pCharacteristic->notify();
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

    
    delay(100);
}

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