/*
  Система сигнализации протечек.
  Имеем 4 зоны контроля и 6 датчиков
  Под мойкой в кухне, под фильтром в кухне, под ванной, 2 под стиральной машиной, 1 в стояке с коммуникациями
  При срабатывании любого датчика включается сигнализация на 30 сек.
  В перспективе подключение к SIM800L для отправки СМС на телефон
  - Подключение датчиков: пины 2-7
  - 7, 8 пины для подключения sim800l
  - 9 пина - пищалка
*/

#include <SoftwareSerial.h>
//#include <LiquidCrystal_I2C.h>

SoftwareSerial sim800l(8, 9); // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
//LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

String SMStext = "Alarm! ";            // текст смс на телефон
String SMStext_zone = "Alarm!2";           // зона протечки
bool SMSflag = false;

int buzz_pin = 9;
unsigned long timer;

//имена зон конотроля
int kitchen = 2;
int water_filter = 3;
int bath = 4;
int washer1 = 5;
int washer2 = 6;
int sewerage = 7;

void SendSMS() {
  Serial.println("Sending SMS...");                  // Печать текста
  sim800l.print("AT+CMGF=1\r");                      // Выбирает формат SMS
  delay(100);
  sim800l.print("AT+CMGS=\"+375295689321\"\r");      // Отправка СМС на указанный номер +792100000000"
  delay(300);
  sim800l.print(SMStext + SMStext_zone);                            // Тест сообщения
  delay(300);
  sim800l.print((char)26);                           // (требуется в соответствии с таблицей данных)
  delay(300);
  sim800l.println();
  Serial.println("Text Sent.");
  SMSflag = true;                                    // поднимаем флаг, во избежание повторных смс
  delay(500);
}

void alarm_on() {
  if (SMSflag == false) {SendSMS();}                 // если смс еще не отпарвлялось, то отправляем
  int alarm_counter = 0;
  while (alarm_counter != 6) {                       // столько будет пищать сигнализация
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
    delay(300);
    alarm_counter += 1;
  }
  delay(1000);
}

void setup() {
  Serial.begin(9600);                         // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Загрузка...");
  sim800l.begin(9600);                        // Инициализация последовательной связи с Arduino и SIM800L
  delay(100);
  //pinMode(buzz_pin, OUTPUT);
  pinMode(13, OUTPUT);
  delay(1000);
  Serial.println("Готово.");
  //lcd.init();
  //lcd.backlight();
}

void loop() {
  switch (digitalRead(kitchen) || \
        digitalRead(water_filter) || \
        digitalRead(bath) || \
        digitalRead(washer1) || \
        digitalRead(washer2) || \
        digitalRead(sewerage)) {
      case HIGH:
        Serial.println("Тревога!");
        alarm_on();
        break;
     }
   delay(100);
}
