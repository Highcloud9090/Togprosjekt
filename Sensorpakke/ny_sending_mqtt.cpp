#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <RTClib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// prototypes
void calibrateSensor(int n);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

// WiFi and MQTT
const char* ssid = "NTNU-IOT";
const char* password = "";
const char* mqtt_server = "192.168.0.140"; //change!
const int mqtt_port = 1883;
const char* mqtt_user = "mjm";
const char* mqtt_pass = "solutions";

WiFiClient espClient;
PubSubClient client(espClient);

// MPU6050
#define SDA_PIN 26
#define SCL_PIN 25

MPU6050 mpu(0x69);// software offsets
float ax_offset = 0, ay_offset = 0, az_offset = 0;
float gx_offset = 0, gy_offset = 0, gz_offset = 0;

RTC_PCF8523 rtc;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--------------------------------------");
  Serial.printf("Serial started (%lu)\n", millis());

  // WiFi and MQTT
  Serial.println("Attempting WiFi connection...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("\nWiFi connected, ip address = ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("\nInitializing Wire");
  Wire.begin(SDA_PIN, SCL_PIN);
  while(!Serial);
  Serial.println("Wire initialized");

  // MPU
  Serial.println("\nInitializing MPU6050");

  Serial.println("MPU6050 Raw + Software Offset Test");

  mpu.initialize();
  delay(500);
  mpu.setSleepEnabled(false);

  Serial.println("MPU6050 started");
  delay(2000);

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not connected!");
    //while (1); delay(10);
  }

  // Sett full-scale range
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);   // ±2g
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);  // ±1000°/s

  Serial.println("Calibrating...");

  calibrateSensor(2500); // 2500 tries
  Serial.println("Calibration done!");

  // RTC
  Serial.println("\nInitializing RTC");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  Serial.println("RTC Initialized");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();

    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

    float axs2 = ax_raw - ax_offset;
    float ays2 = ay_raw - ay_offset;
    float azs2 = az_raw - az_offset;

    float gxdps = gx_raw - gx_offset;
    float gydps = gy_raw - gy_offset;
    float gzdps = gz_raw - gz_offset;

    JsonDocument doc;
    doc["ts"] = rtc.now().unixtime();

    JsonObject accel = doc["accel"].to<JsonObject>();
    accel["x"] = axs2;
    accel["y"] = ays2;
    accel["z"] = azs2;

    JsonObject gyro = doc["gyro"].to<JsonObject>();
    gyro["x"] = gxdps;
    gyro["y"] = gydps;
    gyro["z"] = gzdps;

    char payload[512];
    size_t len = serializeJson(doc, payload, sizeof(payload));
    if (len >= sizeof(payload)) {
        Serial.println("WARNING: payload truncated!");
    } else {
        Serial.printf("Payload size: %u bytes\n", len);
        client.publish("devices/esp32-1", payload, len);
    }

    // Serial.print("payload: ");
    // Serial.println(payload);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

void reconnect() {  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("esp32-1-client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.publish("esp32/topic","hello from esp32");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

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
  az_offset = (az_sum / (float)n) - 16384.0; // remove 1g from Z
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
