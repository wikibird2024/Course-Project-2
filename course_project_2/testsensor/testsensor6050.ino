#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);  // Chân I2C cho ESP32 (SDA = GPIO 21, SCL = GPIO 22)

    if (!mpu.begin()) {
        Serial.println("Không tìm thấy MPU6050!");
    }
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print("X: "); Serial.print(a.acceleration.x);
    Serial.print(" | Y: "); Serial.print(a.acceleration.y);
    Serial.print(" | Z: "); Serial.println(a.acceleration.z);

    delay(100);
}
