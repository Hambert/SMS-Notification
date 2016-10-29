#include <Sim800l.h>
#include <SoftwareSerial.h> //is necesary for the Sim800l library!! 
Sim800l Sim800l; //to declare the library

#include <EEPROM.h>  // using the eeprom memory
/*
        PINOUT:
          _____________________________
         |  ARDUINO UNO >>>   SIM800L  |
          -----------------------------
              GND      >>>   GND
           RX  2       >>>   TX
           TX  3       >>>   RX
         RESET 4       >>>   RST 
*/

String textSms, numberSms;
const String falbackNumber = "";
const int maxSMS = 5;
String recNumber[maxSMS];

uint8_t ArduinoLED = 13; // use what you need
const uint8_t LEDGreen = 5; // LED an Pin 5

int ledState = LOW;
unsigned long previousMillis = 0;
unsigned long prevAskSMS = 0;
const int powerOffTime = 10000;
boolean smsReceived = false;
int eepromAddr = 0;

void setup() {
  // set the digital pin as output:
  pinMode(LEDGreen, OUTPUT);

  digitalWrite(LEDGreen, HIGH);

  Serial.begin(9600); // only for debug the results .
  Serial.println("Begin");
  Sim800l.begin(); // initializate the library.
  Serial.println("Reset");
  Sim800l.reset();
  Serial.println("Reset done");
  delay(100);
  Serial.println("Init done");
  String batt = Sim800l.getBatterieCharge();
  batt = batt.substring(batt.lastIndexOf(",") + 1, batt.lastIndexOf(",") + 5);

  String quali = Sim800l.getSignalQuality();
  quali = quali.substring(quali.lastIndexOf(",") - 2, quali.lastIndexOf(","));

  Serial.println("Send SMS");
  String SmsText = "Signalstaerke:\r\n";
  SmsText += quali;
  SmsText += "\r\nBatterie level\r\n";
  SmsText += batt;
  /* Read sms numbers from EEPROM */
  EEPROM.get(eepromAddr, recNumber);
  
  /* Das nummernarray durchlaufen und für reden Eintrag eine SMS versenden */
  for (int i = 0; i <= maxSMS; i++) {
    if (recNumber[i] == "") {
      if ( i == 0 ) {
        recNumber[i] = falbackNumber;
        Sim800l.sendSms(recNumber[i].c_str(), SmsText.c_str() );
      } else {
        break;
      }
    } else {
      Sim800l.sendSms(recNumber[i].c_str(), SmsText.c_str() );
      delay(100);
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long askSMS = millis();


  /* Blinken bei warten auf SMS */
  if (currentMillis - previousMillis >= 200 && not smsReceived) {

    // save the last time you blinked the LED
    previousMillis = currentMillis;

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LEDGreen, ledState);
  }
  if (smsReceived){
    digitalWrite(LEDGreen,LOW);
    delay(100);

    if (Sim800l.PowerOff()) {
      Serial.print("SIM 800l powered off");
    }
    EEPROM.put(eepromAddr, recNumber);

    eepromAddr = eepromAddr + 1;
    if (eepromAddr == EEPROM.length()) {
      eepromAddr = 0;
    }
  
  }

  if ( not smsReceived && askSMS - prevAskSMS >= 1000) {
    prevAskSMS = askSMS;
    textSms = Sim800l.readSms(1); //read the first sms
  }

  if (textSms.indexOf("OK") != -1) //first we need to know if the messege is correct. NOT an ERROR
  {
    if (textSms.length() > 7)  // optional you can avoid SMS empty
    {

      numberSms = Sim800l.getNumberSms(1); // Here you have the number
      //for debugin
      Serial.print("SMS von: ");
      Serial.println(numberSms);
      Serial.print("SMS Text: ");
      Serial.println(textSms);
      textSms.toUpperCase();  // set all char to mayus ;)

      int idx = 0;
      int plusPos = 0;
      int startPos = textSms.lastIndexOf('"');
      int stPlus = startPos;
      int ndPlus = stPlus + 2;

      do {
        stPlus = textSms.indexOf("+", stPlus + 1);
        ndPlus = textSms.indexOf("+", stPlus + 2);
        if (stPlus == -1){
          break;
          // SMS enthält kein + an der richtigen stelle
        }
        /* validate Number */
        int j = stPlus + 1;
        while ( isDigit(textSms.charAt(j)) ) {
          j++;
        }
        ndPlus = j; // Bei minus 1 wird er nicht geschrieben


        Serial.println("-------------------------");
        Serial.println(stPlus);
        Serial.println(ndPlus);
        Serial.println("-------------------------");

        /* Extrahiere die Nummer aus dem Text */

        recNumber[idx] = textSms.substring(stPlus, ndPlus);

        
        

/* DEBUG */
        Serial.print("SMS ");
        Serial.print(idx);
        Serial.print(": ");
        Serial.println(recNumber[idx]);
        smsReceived = true;

        if ( textSms.charAt( ndPlus + 1 ) == '\n' || textSms.charAt( ndPlus + 1 ) == '\r'){
          break;
        }

        idx ++;
      } while (idx <= 5);

      String SmsText = "Folgende Nummer(n) wurden eingetragen:\n\n";
      for (int i = 0; i <= maxSMS; i++) {
        SmsText += recNumber[i];
        SmsText += "\n";
      }
      textSms = "";
      //Sim800l.sendSms(numberSms.c_str(), SmsText.c_str() );

    }
    delay(200);
    Sim800l.delAllSms(); //do only if the message is not empty,in other case is not necesary
    Serial.print("done");
    //delete all sms..so when receive a new sms always will be in first position

  }
}

