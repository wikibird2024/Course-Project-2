#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
HardwareSerial gpsSerial(2); // Serial2 cho module GPS (TX 16, RX 17)

// Các biến phát hiện té ngã
const float threshold = 18.0; // Ngưỡng gia tốc để phát hiện té ngã
unsigned long fallStartTime = 0;
bool fallDetected = false;

// Số điện thoại nhận tin nhắn
String phoneNumber = "+84xxxxxxxxx"; // Thay sdt nhan tin nhan

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // UART giao tiếp với module 4G-GPS

  // Khởi tạo MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 không tìm thấy!");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG); // set gia tri cua MPU 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Bật GPS trên module
  initGPS();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Tính tổng gia tốc
  float totalAccel = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

  // Phát hiện té ngã
  if (totalAccel > threshold) {
    if (!fallDetected) {
      fallDetected = true;
      fallStartTime = millis();
    }
  } else {
    if (fallDetected && (millis() - fallStartTime > 2000)) { // Kiểm tra trạng thái té ngã trong 2 giây
      fallDetected = false;
    }
  }

  // Gửi tin nhắn khi phát hiện té ngã
  if (fallDetected) {
    sendSMSWithLocation();
    delay(60000); // Tránh gửi tin nhắn liên tục
  }
}

void initGPS() {
  sendATCommand("AT+QGPS=1", 1000); // Bật GPS
}

String getGPSLocation() {
  sendATCommand("AT+QGPSLOC=2", 5000); // Lấy vị trí GPS
  String gpsData = "";
  long startTime = millis();
  while ((millis() - startTime) < 5000) { // Chờ dữ liệu tối đa 5 giây
    while (gpsSerial.available()) {
      gpsData += (char)gpsSerial.read();
    }
  }
  return gpsData;
}

void sendSMSWithLocation() {
  String location = getGPSLocation();
  if (location.length() < 5) {
    Serial.println("Không lấy được vị trí GPS!");
    return;
  }
  
  String message = "Phát hiện té ngã! Vị trí: " + location;

  sendATCommand("AT+CMGF=1", 1000); // Đặt chế độ SMS text
  sendATCommand("AT+CMGS=\"" + phoneNumber + "\"", 1000);
  gpsSerial.println(message);
  delay(100);
  gpsSerial.print((char)26); // Gửi SMS với Ctrl+Z
  Serial.println("SMS đã gửi!");
}

void sendATCommand(String command, int timeout) {
  gpsSerial.println(command);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (gpsSerial.available()) {
      Serial.write(gpsSerial.read());
    }
  }
  Serial.println();
}
