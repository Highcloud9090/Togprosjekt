#include <Wire.h>
#include <Arduino.h>

#define MPU_ADDR 0x68

#define I2C_SDA 33
#define I2C_SCL 32

int16_t off_ax, off_ay, off_az;


void readRawMPU(int16_t &ax, int16_t &ay, int16_t &az,
                int16_t &gx, int16_t &gy, int16_t &gz) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  Wire.read(); Wire.read(); // temperatur (ignorer)

  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}


void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println(F("MPU6050 test"));
  Wire.begin(I2C_SDA, I2C_SCL);

  // Vekk MPU6050 (slå av sleep)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);      // PWR_MGMT_1
  Wire.write(0x00);      // Wake up
  Wire.endTransmission();

  Serial.println("MPU6050 RAW data");

  //kaliberingssjekk
  int tot_ax = 0;
  int tot_ay = 0;
  int tot_az = 0;
  int itr = 100;

  for (int i = 0; i < itr; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    readRawMPU(ax, ay, az, gx, gy, gz);
    
    tot_ax += ax;
    tot_ay += ay;
    tot_az += az;
    delay(15);
  }

  int S_ax = 0;
  int S_ay = 0;
  int S_az = 0;

  S_ax = tot_ax/itr;
  S_ay = tot_ay/itr;
  S_az = tot_az/itr;

  off_ax = S_ax;
  off_ay = S_ay;
  off_az = S_az - 16384; // viktig: fjern 1g på Z

  Serial.print("AX offset: "); Serial.println(off_ax);
  Serial.print("AY offset: "); Serial.println(off_ay);
  Serial.print("AZ offset: "); Serial.println(off_az);


}

void loop() {

  /*

  Serial.print("Accel RAW  X: ");
  Serial.print(ax);
  Serial.print("  Y: ");
  Serial.print(ay);
  Serial.print("  Z: ");
  Serial.println(az);

  Serial.print("Gyro  RAW  X: ");
  Serial.print(gx);
  Serial.print("  Y: ");
  Serial.print(gy);
  Serial.print("  Z: ");
  Serial.println(gz);

  Serial.println();
  delay(3000);
  */

}
