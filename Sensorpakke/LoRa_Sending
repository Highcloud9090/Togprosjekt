#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#define RFM_CS   33
#define RFM_RST  32
#define RFM_DIO0 14

#define SPI_SCK  27
#define SPI_MISO 26
#define SPI_MOSI 25

SX1276 radio = new Module(RFM_CS, RFM_DIO0, RFM_RST, -1);
int teller = 0;

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

}

void loop() {
  
  int state = radio.transmit("P " + String(teller));

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Pakke sendt " + String(teller));
  } else {
    Serial.print("Sending feilet");
    Serial.println(state);
  }
  teller += 1;
  delay(2000);

}
