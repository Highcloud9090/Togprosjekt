#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebServer.h>

// prototypes
void printValues();
String getValues();

const char* ssid = "NTNU-IOT"; // NTNU-IOT,
const char* password = "";   

WebServer server(80);

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS);

void handleRoot() {
  const char* html = R"rawliteral(
    <!doctype html>
    <html>
    <head><meta charset="utf-8"><title>ESP32 BME280</title></head>
    <body>
      <h3>ESP32 BME280 sensor</h3>
      <div><strong>Temperature: </strong> <span id="temp">--</span> °C</div>
      <div><strong>Pressure: </strong> <span id="pres">--</span> hPa</div>
      <div><strong>Humidity: </strong> <span id="humi">--</span> %</div>

      <script>
        async function update() {
          try {
            const r = await fetch('/state');
            const j = await r.json();
            document.getElementById('temp').textContent = parseFloat(j.temperature).toFixed(2);
            document.getElementById('pres').textContent = parseFloat(j.pressure).toFixed(2);
            document.getElementById('humi').textContent = parseFloat(j.humidity).toFixed(2);
            } catch (e) { console.error(e); }
        }
        update();
        setInterval(update, 2000);
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void sendState() {
  String payload = String("{") + getValues() + String("}");
  server.send(200, "application/json", payload);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // BME280 sensor (SPI)
  Serial.println("BME280 SPI test");
  SPI.begin(BME_SCK, BME_MISO, BME_MOSI, BME_CS);

  if (!bme.begin(BME_CS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) { delay(1000); }
  }

  Serial.println("-- Default Test --");
  printValues();

  // server and Wi-Fi
  Serial.println("Starting server...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi connected, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("WiFi connection failed or timed out");
  }

  server.on("/", handleRoot);
  server.on("/state", sendState);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

String getValues() {
  String s;
  s += "\"temperature\":";
  s += String(bme.readTemperature(), 2);
  s += ",\"pressure\":";
  s += String(bme.readPressure() / 100.0F, 2);
  s += ",\"humidity\":";
  s += String(bme.readHumidity(), 2);
  return s;
}
