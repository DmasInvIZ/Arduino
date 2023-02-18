#include <SoftwareSerial.h>
#include <Sim800l.h>

Sim800l Sim800l;
bool sending;

String text;
uint8_t index;

SoftwareSerial SIM800(8, 9);        // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

String textSms,numberSms;
uint8_t index1;
uint8_t LED2=13; // use what you need 
bool error;
String on = "TURNON";
String off = "TURNOFF";

void setup() {
  Serial.begin(9600);               // Скорость обмена данными с компьютером
  Serial.println("Start!");
  SIM800.begin(9600);               // Скорость обмена данными с модемом
  SIM800.println("AT");

  Sim800l.begin(); // initializate the library. 
//  sending = Sim800l.sendSms("+375295689321", "Text");
//  Serial.println(sending);

}
void loop() {
  Serial.println("scan");
  Serial.println(textSms);
  textSms=Sim800l.readSms(1);                                //read the first sms
    if (textSms.indexOf("OK")!=-1) {                         //first we need to know if the messege is correct. NOT an ERROR
      numberSms=Sim800l.getNumberSms(1);                     // Here you have the number
      Serial.println(numberSms); 
      textSms.toUpperCase();                                 // set all char to mayus ;)
      if (textSms.indexOf(on)!=-1) {
          Serial.println("LED TURN ON");
          digitalWrite(LED2, 1);
          sending = Sim800l.sendSms("+375295689321", "ON");
      }
      else if (textSms.indexOf(off)!=-1) {
          Serial.println("LED TURN OFF");
          digitalWrite(LED2, 0);
          sending = Sim800l.sendSms("+375295689321", "OFF");
      }
      else{
          Serial.println("Not Compatible ...sorry.. :D");
      }
      Sim800l.delAllSms();
    }
    delay(1000);
  }
