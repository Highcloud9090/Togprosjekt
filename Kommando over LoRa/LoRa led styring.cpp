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


//-----------SEND COMMAND--------------
bool buttonPressed = false;


void setup() {#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#define RFM_CS   33
#define RFM_RST  32
#define RFM_DIO0 14

#define SPI_SCK  27
#define SPI_MISO 26
#define SPI_MOSI 25

SX1276 radio = new Module(RFM_CS, RFM_DIO0, RFM_RST, -1);

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
}


  pinMode(13, INPUT_PULLDOWN);
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
  buttonPressed = digitalRead(13);
  Serial.println(buttonPressed);
   int state = radio.transmit(""+String(buttonPressed));


  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Pakke sendt");
  } else {
    Serial.print("Sending feilet");
    Serial.println(state);
  }
  delay(2000);
}


//-----------RECEIVE COMMAND--------------
bool ledState = false;


void setup() {
 pinMode(13, OUTPUT);
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
  String data;
 int state = radio.receive(data);
  if (state == RADIOLIB_ERR_NONE) {
   Serial.println("Pakke motatt");
   Serial.println(String(data));
   Serial.println("-------------------");
   ledState  = (data == "0") ? false : true;
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
 digitalWrite(13, ledState);
}

