#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme(BME_CS);

//  prototypes
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

const char* ssid = "NTNU-IOT";
const char* password = "";
const char* mqtt_server = "10.22.57.3"; // Docker host IP
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

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
    if (client.connect("esp32-client")) {
      Serial.println("connected");
      client.subscribe("other/topic");
      client.publish("esp32/topic","hello from esp32");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

String getSensorValues() {
  String s;
  s += "\"temperature\":";
  s += String(bme.readTemperature(), 2);
  s += ",\"pressure\":";
  s += String(bme.readPressure() / 100.0F, 2);
  s += ",\"humidity\":";
  s += String(bme.readHumidity(), 2);
  return s;
}

void setup() {
  Serial.begin(115200);

  Serial.println("BME280 SPI test");
  SPI.begin(BME_SCK, BME_MISO, BME_MOSI, BME_CS);

  if (!bme.begin(BME_CS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) { delay(1000); }
  }

  Serial.println("Attempting WiFi connection...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("\nWiFi connected, ip address = ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    String payload = getSensorValues();
    client.publish("esp32/topic", payload.c_str());
  }
}
