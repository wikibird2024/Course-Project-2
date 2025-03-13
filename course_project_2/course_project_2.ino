#define TINY_GSM_MODEM_SIM800  
#include <TinyGsmClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
HardwareSerial gpsSerial(2);  // Serial2 cho module 4G-GPS (TX=16, RX=17)

// Biến phát hiện té ngã
const float threshold = 18.0;  // Ngưỡng gia tốc để phát hiện té ngã
unsigned long fallStartTime = 0;
bool fallDetected = false;
bool smsSent = false;  // Kiểm soát gửi SMS

// Số điện thoại nhận tin nhắn
const String phoneNumber = "+84559865843";  // Thay số điện thoại nhận SMS

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // UART giao tiếp với module 4G-GPS

    // Khởi tạo MPU6050
    if (!mpu.begin()) {
        Serial.println("MPU6050 không tìm thấy!");
        while (1);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    initGPS();
}

void loop() {
    detectFall();
}

void detectFall() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float totalAccel = sqrt(a.acceleration.x * a.acceleration.x +
                            a.acceleration.y * a.acceleration.y +
                            a.acceleration.z * a.acceleration.z);

    if (totalAccel > threshold) {
        if (!fallDetected) {
            fallDetected = true;
            fallStartTime = millis();
            smsSent = false;  // Reset trạng thái gửi SMS
        }
    } else {
        if (fallDetected && (millis() - fallStartTime > 2000)) {
            fallDetected = false;
        }
    }

    // Gửi tin nhắn nếu phát hiện té ngã và chưa gửi SMS trước đó
    if (fallDetected && !smsSent) {
        sendSMSWithLocation();
        smsSent = true;
    }
}

void initGPS() {
    sendATCommand("AT+QGPS=1", 1000);
}

String getGPSLocation() {
    sendATCommand("AT+QGPSLOC=2", 5000);

    String gpsData = "";
    long startTime = millis();
    while ((millis() - startTime) < 5000) {
        while (gpsSerial.available()) {
            char c = gpsSerial.read();
            gpsData += c;
        }
    }

    int pos = gpsData.indexOf("+QGPSLOC:");
    if (pos != -1) {
        return gpsData.substring(pos);
    } else {
        return "Không lấy được vị trí GPS!";
    }
}

void sendSMSWithLocation() {
    String location = getGPSLocation();
    String message = "Phát hiện té ngã! Vị trí: " + location;

    sendATCommand("AT+CMGF=1", 1000);
    sendATCommand("AT+CMGS=\"" + phoneNumber + "\"", 1000);
    
    gpsSerial.print(message);
    gpsSerial.write(26);  // Ctrl+Z để gửi SMS
    Serial.println("SMS đã gửi!");
}

void sendATCommand(String command, int timeout) {
    gpsSerial.println(command);
    long int time = millis();
    while ((millis() - time) < timeout) {
        while (gpsSerial.available()) {
            Serial.write(gpsSerial.read());
        }
    }
    Serial.println();
}
