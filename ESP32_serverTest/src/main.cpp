#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "NTNU-IOT";
const char* password = "";

WebServer server(80);
const uint8_t PIN = 25;

void sendState(){
  int val = digitalRead(PIN);
  String payload = String("{\"Is button pressed\":") + PIN + String(",\"value\":") + val + String("}");
  server.send(200, "application/json", payload);
}

void handleSet(){
  server.send(400, "text/plain", "Pin is input-only\n"); // don't allow writes to input pin
}

void handleRoot(){
  String html = "<!doctype html><html><body>"
    "<h3>ESP32 GPIO</h3>"
    "<p>Pin " + String(PIN) + " = <span id=\"val\">?</span></p>"
    "<script>"
      "const out=document.getElementById('val');"
      "const es=new EventSource('/events');"
      "es.onmessage = e => { try{ const j=JSON.parse(e.data); out.textContent = j.value; } catch(_){} };"
      "es.onerror = e => console.log('SSE error', e);"
    "</script>"
    "</body></html>";
  server.send(200, "text/html", html);
}


void handleEvents(){
  WiFiClient client = server.client();
  // send SSE headers
  client.print(F("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/event-stream\r\n"
                 "Cache-Control: no-cache\r\n"
                 "Connection: keep-alive\r\n\r\n"));
  unsigned long last = millis();
  while (client.connected()) {
    // allow other server tasks
    server.handleClient();
    if (millis() - last >= 500) {
      last = millis();
      int val = digitalRead(PIN);
      String msg = String("data: {\"pin\":") + PIN + String(",\"value\":") + val + String("}\n\n");
      if (client.connected()) {
        client.print(msg);
      } else break;
    }
    delay(10);
  }
}


void setup(){
  Serial.begin(115200);
  delay(100);
  // Make GPIO25 an input with internal pulldown
  pinMode(PIN, INPUT_PULLDOWN);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.printf("WiFi status=%d, IP=%s\n", WiFi.status(), WiFi.localIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/state", sendState);
  server.on("/set", handleSet);
  server.on("/events", handleEvents);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(){
  server.handleClient();
}
