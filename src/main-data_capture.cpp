
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <ESP32Time.h>


MMA8452Q accel;
ESP32Time rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (accel.begin() == false) {
    Serial.println("Not Connected. Please check connections and read the hookup guide.");
    while (1);
  }
}

void loop() {
  if (Serial.available() > 0) {
    if (Serial.read() == 'l') {
      // Orientation of board (Right, Left, Down, Up);
      Serial.print(rtc.getTime());
      Serial.print(", ");
      Serial.print(accel.getX());
      Serial.print(", ");
      Serial.print(accel.getY());
      Serial.print(", ");
      Serial.print(accel.getZ());
      Serial.println();
    }
  }
}
