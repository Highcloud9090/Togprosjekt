#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define RFM_CS   33
#define RFM_RST  32
#define RFM_DIO0 14

#define SPI_SCK  27
#define SPI_MISO 26
#define SPI_MOSI 25

SX1276 radio = new Module(RFM_CS, RFM_DIO0, RFM_RST, -1);

//  prototypes
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

const char* ssid = "NTNU-IOT";
const char* password = "";
const char* mqtt_server = "10.22.102.159"; // Linux host IP
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
  Serial.println("Starter program");

  //Definerer pins og starter kommunikasjon
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, RFM_CS);

  //stiller inn senderinnstillinger
  int state = radio.begin(868.0);
  radio.setOutputPower(20);
  radio.setSpreadingFactor(12);
  radio.setBandwidth(125.0);
  radio.setCodingRate(5); 


  //hvis man mister kontakt med radioen
  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("Radio feilet");
    while (true);
  }
  delay(1000);

  Serial.println("Attempting WiFi connection...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("\nWiFi connected, ip address = ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  
  String data;
  // Serial.println("Før receive");
  int state = radio.receive(data);
  // Serial.println("Etter receive");
  
  if (state == RADIOLIB_ERR_NONE) {

    int16_t RSSI = radio.getRSSI();
    int16_t SNR = radio.getSNR();

    Serial.println(String(data));
    Serial.println("RSSI = " + String(RSSI));
    Serial.println("SNR = " + String(SNR));
    Serial.println("-------------------");
  } 
  else {
    int state = radio.receive(data);
    switch (state) {
      case RADIOLIB_ERR_RX_TIMEOUT:
      Serial.println("Feil: Timeout (ingen pakke)");
        break;

      case RADIOLIB_ERR_CRC_MISMATCH:
      Serial.println("Feil: CRC mismatch (korrupt pakke)");
        break;

      case RADIOLIB_ERR_WRONG_MODEM:
      Serial.println("Feil: Wrong modem (ikke LoRa-signal)");
        break;

      case RADIOLIB_ERR_INVALID_FREQUENCY:
      Serial.println("Feil: Invalid frequency (kode 7)");
        break;

    default:
      Serial.print("Ukjent feil: ");
      Serial.println(state);
      break;
    }
  }

  if (!client.connected()) reconnect();
  client.loop();
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    String payload = data;
    client.publish("esp32/topic", payload.c_str());
  }

}
