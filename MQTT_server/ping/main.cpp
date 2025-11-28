#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>


//  prototypes
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

const char* ssid = "NTNU-IOT";
const char* password = "";
const char* mqtt_server = "10.22.58.98"; // Docker host IP
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

void setup() {
  Serial.begin(115200);
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
    client.publish("esp32/topic", "ping");
  }
}
