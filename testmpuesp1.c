#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "MPU6050.h"

// GPIO setup for the buzzer
#define BUZZER_PIN 21

// I2C setup for MPU6050
#define I2C_MASTER_SCL 22
#define I2C_MASTER_SDA 21
#define I2C_MASTER_NUM 0

MPU6050 mpu;

void app_main() {
    // Initialize GPIO for buzzer
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    
    // Initialize I2C
    i2c_master_init();
    mpu.initialize();

    // Variables to store sensor data
    float ax, ay, az;
    float gx, gy, gz;

    while (1) {
        // Get accelerometer and gyroscope data
        mpu.getAcceleration(&ax, &ay, &az);
        mpu.getRotation(&gx, &gy, &gz);

        // Check for fall based on acceleration threshold
        if (az < -9.8) {
            // Trigger buzzer if fall is detected
            gpio_set_level(BUZZER_PIN, 1); // Turn buzzer on
            vTaskDelay(500 / portTICK_PERIOD_MS); // Keep buzzer on for 500ms
            gpio_set_level(BUZZER_PIN, 0); // Turn buzzer off
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100ms
    }
}
