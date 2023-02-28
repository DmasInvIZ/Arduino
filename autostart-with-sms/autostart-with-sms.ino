#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>

#define BTN_PIN 2                             // кнопка остановки двигателя
#define pin_ACC 4                             // первое положение ключа
#define pin_ENGINE 5                          // зажигание
#define pin_START 6                           // стартер
#define pin_break                             // ручной тормоз
#define pin_neutral                           // коробка передач, нейтраль
#define temp A0                               // датчик температуры
#define tel "+375295689321"                   // реагируем на смс только с этого номера

EncButton<EB_TICK, BTN_PIN> enc;
Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                   // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

String textSms,numberSms;                      // текст смс и номер абонента

void setup() {
  Serial.begin(9600);     // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  SIM800.println("AT");
  Sim800l.begin();                            // Инициализация модема
  pinMode(BTN_PIN, INPUT_PULLUP);
  Sim800l.delAllSms();
  Serial.println("All sms deleted");
  Serial.println("Ready");
}

void loop() {
  textSms=Sim800l.readSms(1);
  if (textSms.indexOf("OK")!=-1) {
    numberSms=Sim800l.getNumberSms(1);
    Serial.println(numberSms);
    if (numberSms == tel) {
      Serial.println("this number!");
      textSms.toUpperCase();
      if (textSms.indexOf("\nSTART\r\n")!=-1) {
        Serial.println("start!");
        Sim800l.sendSms(tel, "started!");
      }
      else {
        Serial.println("wrong phrase");
      }
    }
    Sim800l.delAllSms();
    Serial.println("del sms");
  }
  
}
