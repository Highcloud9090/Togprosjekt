#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <RTClib.h>

// ---------------- PIN SETUP ----------------

// MPU6050
#define SDA_PIN 21
#define SCL_PIN 18

// SD-kort
#define SD_CS   25
#define SD_SCK  26
#define SD_MISO 34
#define SD_MOSI 27

// ---------------- OBJEKTER ----------------

Adafruit_MPU6050 mpu;
RTC_PCF8523 rtc;
File logFile;

// ---------------- BUFFER ----------------

#define BUFFER_SIZE 25
char buffer[BUFFER_SIZE][120];  // fast buffer
int bufferIndex = 0;
int flushCounter = 0;

// ---------------- TIMING ----------------

const int sampleInterval = 100; // 10 Hz
unsigned long lastSample = 0;

// ---------------- RAW LESING ----------------

void readRawMPU(int16_t &ax, int16_t &ay, int16_t &az,
                int16_t &gx, int16_t &gy, int16_t &gz) {

  Wire.beginTransmission(0x69);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x69, 14, true);

  if (Wire.available() < 14) return;

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read(); // temp
  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();
}

// ---------------- KALIBRERING ----------------

int16_t ax_offset = 0, ay_offset = 0, az_offset = 0;
int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;

void calibrateSensor(int n) {
  long ax_sum = 0, ay_sum = 0, az_sum = 0;
  long gx_sum = 0, gy_sum = 0, gz_sum = 0;

  int16_t ax, ay, az, gx, gy, gz;

  Serial.println("Kalibrerer... hold sensoren i ro lil bitch!");

  for (int i = 0; i < n; i++) {
    readRawMPU(ax, ay, az, gx, gy, gz);

    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    gx_sum += gx;
    gy_sum += gy;
    gz_sum += gz;

    delay(5);
  }

  ax_offset = ax_sum / n;
  ay_offset = ay_sum / n;
  az_offset = (az_sum / n) - 16384; // 1g på Z

  gx_offset = gx_sum / n;
  gy_offset = gy_sum / n;
  gz_offset = gz_sum / n;

  Serial.println("Offsets:");
  Serial.print(" AX: "); Serial.print(ax_offset);
  Serial.print(" AY: "); Serial.print(ay_offset);
  Serial.print(" AZ: "); Serial.println(az_offset);

  Serial.print(" GX: "); Serial.print(gx_offset);
  Serial.print(" GY: "); Serial.print(gy_offset);
  Serial.print(" GZ: "); Serial.println(gz_offset);
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);
  delay(500); // viktig for Serial

  //START I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // RTC
  if (!rtc.begin()) {
    Serial.println("RTC feil");
    while (1);
  }
  delay(200);

  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  delay(200);

  // SD
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(200);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD-kort feilet!");
    while (1);
  }
  delay(200);

  logFile = SD.open("/data.csv", FILE_APPEND);
  if (!logFile) {
    Serial.println("Kunne ikke åpne fil!");
    while (1);
  }
  delay(200);

  if (logFile.size() == 0) {
    logFile.println("teller,timestamp,ax,ay,az,gx,gy,gz");
    logFile.flush();
  }

  Serial.println("SD-kort klart");
  delay(200);

  // MPU

  if (!mpu.begin(0x69)) {
    Serial.println("MPU6050 feil!");
    while (1);
  }
  delay(200);

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_1000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(200);

  calibrateSensor(2500);

  Serial.println("Klar!");
}

// ---------------- WRITE BUFFER ----------------

void writeBufferToSD() {
  if (!logFile) return;

  for (int i = 0; i < bufferIndex; i++) {
    logFile.println(buffer[i]);
  }

  bufferIndex = 0;

  flushCounter++;
  if (flushCounter >= 5) {
    logFile.flush();
    flushCounter = 0;
    Serial.println("Flush");
  }
}

// ---------------- LOOP ----------------

int teller = 0;

void loop() {
  if (millis() - lastSample >= sampleInterval) {
    lastSample = millis();

    int16_t ax, ay, az, gx, gy, gz;
    readRawMPU(ax, ay, az, gx, gy, gz);

    // Tid
    DateTime now = rtc.now();
    char timestamp[25];

    if(now.year() >= 2020 && now.year() <= 2030){
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
    }
    else{
    sprintf(timestamp, "non");
    }

    // CSV linje
    sprintf(buffer[bufferIndex],
            "%d,%s,%d,%d,%d,%d,%d,%d",
            teller, timestamp,
            ax - ax_offset,
            ay - ay_offset,
            az - az_offset,
            gx - gx_offset,
            gy - gy_offset,
            gz - gz_offset);

    bufferIndex++;

    if (bufferIndex >= BUFFER_SIZE) {
      writeBufferToSD();
    }

    Serial.println(buffer[bufferIndex - 1]);

    teller++;
  }
}
