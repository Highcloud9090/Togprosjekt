#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Arduino.h>

#define SDA_PIN 33
#define SCL_PIN 32

MPU6050 mpu;

// Software offsets
float ax_offset = 0, ay_offset = 0, az_offset = 0;
float gx_offset = 0, gy_offset = 0, gz_offset = 0;
int16_t mnr = 0;


// Funksjon for å kalibrere sensor
void calibrateSensor(int n) {
  long ax_sum = 0, ay_sum = 0, az_sum = 0;
  long gx_sum = 0, gy_sum = 0, gz_sum = 0;

  int16_t ax, ay, az, gx, gy, gz;

  for (int i = 0; i < n; i++) {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    gx_sum += gx;
    gy_sum += gy;
    gz_sum += gz;
    delay(5);
  }

  ax_offset = ax_sum / (float)n;
  ay_offset = ay_sum / (float)n;
  az_offset = (az_sum / (float)n) - 16384.0; // trekk fra 1g for Z
  gx_offset = gx_sum / (float)n;
  gy_offset = gy_sum / (float)n;
  gz_offset = gz_sum / (float)n;

  Serial.println("Offsets beregnet:");
  Serial.print("Accel X: "); Serial.print(ax_offset);
  Serial.print("  Y: "); Serial.print(ay_offset);
  Serial.print("  Z: "); Serial.println(az_offset);

  Serial.print("Gyro X: "); Serial.print(gx_offset);
  Serial.print("  Y: "); Serial.print(gy_offset);
  Serial.print("  Z: "); Serial.println(gz_offset);
}


void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);
  while (!Serial);

  Serial.println("MPU6050 Raw + Software Offset Test");

  mpu.initialize();
  Serial.println("Starter MPU6050");
  delay(2000);

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 ikke koblet til!");
    while (1);
  }

  // Sett full-scale range
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);   // ±2g
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);  // ±1000°/s

  Serial.println("Kalibrerer...");

  calibrateSensor(2500); // ta 2500 prøver
  Serial.println("Kalibrering ferdig!");
}

void loop() {
  int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
  mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

  // Konverter til m/s² og juster med offset
  float axs2 = (ax_raw - ax_offset); // 16384.0 * 9.80665;
  float ays2 = (ay_raw - ay_offset); // 16384.0 * 9.80665;
  float azs2 = (az_raw - az_offset); // 16384.0 * 9.80665;

  // Gyro i deg/s
  float gxdps = (gx_raw - gx_offset) / 32.8;
  float gydps = (gy_raw - gy_offset) / 32.8;
  float gzdps = (gz_raw - gz_offset) / 32.8;

  Serial.print("Måling: "); Serial.println(mnr);

  Serial.print("Accel X: "); Serial.print(axs2);
  Serial.print("  Y: "); Serial.print(ays2);
  Serial.print("  Z: "); Serial.println(azs2);
  Serial.print("Gyro X: "); Serial.print(gxdps);
  Serial.print("  Y: "); Serial.print(gydps);
  Serial.print("  Z: "); Serial.println(gzdps);

  Serial.println();

  mnr += 1;
  delay(500);
}
