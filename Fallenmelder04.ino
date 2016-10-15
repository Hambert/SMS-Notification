#include <Sim800l.h>
#include <SoftwareSerial.h> //is necesary for the Sim800l library!! 
Sim800l Sim800l; //to declare the library

/*
 *      PINOUT:
 *        _____________________________
 *       |  ARDUINO UNO >>>   SIM800L  |
 *        -----------------------------
 *            GND      >>>   GND
 *         RX  2       >>>   TX
 *         TX  3       >>>   RX
 *       RESET 4       >>>   RST 
 */

String textSms, numberSms;

uint8_t ArduinoLED = 13; // use what you need
const uint8_t LEDGreen = 5; // LED an Pin 5

boolean blinkOn = true;

int ledState = LOW;
unsigned long previousMillis = 0;
const int powerOffTime = 10000;

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
  //Sim800l.sendSms("+4915140118278", SmsText.c_str() );
  Serial.println("Ende");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 500 && blinkOn) {

    // save the last time you blinked the LED
    previousMillis = currentMillis;


    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LEDGreen, ledState);
  }
  
  textSms = Sim800l.readSms(1); //read the first sms

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
      int plusPos=0;
      String recNumber[5];
      int stPlus = textSms.indexOf("\n",textSms.indexOf("\n") + 1);
      int ndPlus = stPlus + 2;
      
      do{
        stPlus = textSms.indexOf("+", stPlus + 1);
        ndPlus = textSms.indexOf("+", stPlus + 2);
        Serial.println("-------------------------");
        Serial.println(stPlus);
        Serial.println(ndPlus);
        Serial.println("-------------------------");
        
        if (ndPlus != -1){
           numberSms = textSms.substring(stPlus,ndPlus);
        }
        else if (stPlus != -1)
        {
          numberSms = textSms.substring(stPlus);
        }
        else {
          break;
        }
        recNumber[idx] = numberSms;
        
        Serial.print("SMS ");
        Serial.print(idx);
        Serial.print(": ");
        Serial.println(recNumber[idx]);
        
        idx ++;  
      } while(idx <= 5);
    }
    Sim800l.delAllSms(); //do only if the message is not empty,in other case is not necesary
    //delete all sms..so when receive a new sms always will be in first position
  }
}

