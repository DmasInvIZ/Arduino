/*
  Система сигнализации протечек.
  Имеем 5 зон контроля и 6 датчиков
  Под мойкой в кухне, под фильтром в кухне, под ванной, 2 под стиральной машиной, 1 в стояке с коммуникациями
  При срабатывании любого датчика включается сигнализация на 20 сек.
  После того как обнаружена протечка включается пищалка, отсылается смс на указанные номера 
  информация с зоной протечки выводится на дисплей
  Работа алгоритма прекращается до перезагрузки МК.

  - Подключение датчиков: пины 2-7
  - 7, 8 пины для подключения sim800l
  - 11 пин - пищалка
  - A4, A5 - дисплей

  В следующих версиях прошивки:
  работы программы не останавливается после обнаружения протечки
*/

#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>
#include <LiquidCrystal_I2C.h>

#define buzz_pin 13                                           // пин сигналки   
#define BTN_PIN 3                                             // кнопка теста
#define SMStext "Alarm! Zone: "                               // текст смс на телефон
#define tel_number "+375295689321"                            // номер телефона для отправки
#define tel_number2 "+375257769952"                           // 2 номер телефона для отправки
#define alarm_sec 2                                           // пищим столько раз (секунд)
#define test_alarm_sec 2                                      // пищим столько раз (секунд) для теста сигнализации

EncButton<EB_TICK, BTN_PIN> enc;

Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

LiquidCrystal_I2C lcd(0x27, 16, 2);                           // инициализируем переменную lcd


// имена зон конотроля
#define kitchen         2
#define water_filter    3
#define bath            4
#define washer1         5
#define washer2         6
#define sewerage        7


// функция пищания сигнализации (добавить время работы сигнала)
void alarming(int blink_time) {
  while (blink_time > 0) {
    digitalWrite(buzz_pin, HIGH);
    delay(500);
    digitalWrite(buzz_pin, LOW);
    delay(500);
    blink_time -= 1;
  }
}

// тест 
void alarm_test() {
  Serial.println("alarm test");
  //////////////////////////////////////////////
  alarming(3);
  Sim800l.sendSms(tel_number, "Test SMS, module OK");
  Serial.println("send sms");
}

void SendSMS(String SMStext_zone) {
  Sim800l.sendSms(tel_number, SMStext + SMStext_zone);
  //Sim800l.sendSms(tel_number2, SMStext + SMStext_zone);
  Serial.println("alarm message sent");
}

void alarm_on(zone, text) {
  
}

void setup() {
  Serial.begin(9600);     // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  Sim800l.begin();                            // Инициализация модема
  SIM800.println("AT");
  pinMode(buzz_pin, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);             // пин кнопки с внутренней подтяжкой
  delay(1000);
  Serial.println("Ready.");
}

void loop() {
  if (digitalRead(kitchen) == HIGH) {
    alarm_on(kitchen, "Alarm! Zone: kitchen");
  }
  if (digitalRead(water_filter) == HIGH) {
    alarm_on(water_filter, "Alarm! Zone: kitchen, water filter");
  }
  if (digitalRead(bath) == HIGH) {
    alarm_on(bath, "Alarm! Zone: bathroom, under bath");
  }
  if (digitalRead(washer1) == HIGH || digitalRead(washer2) == HIGH) {
    alarm_on(washer, "Alarm! Zone: washing machine");
  }
  if (digitalRead(sewerage) == HIGH) {
    alarm_on(sewerage, "Alarm! Zone: sewerage, bathroom, behind the toilet");
  }
  //butt1.tick();
  enc.tick();
  //if (butt1.isSingle()) {
  if (enc.hasClicks()) {
    alarm_test();
    //Sim800l.sendSms(tel_number, "Test SMS, module OK");
    //Sim800l.sendSms(tel_number2, "Test SMS, module OK");
    //Serial.println("send sms");                                    // по нажатию кнопки отправляем тестовое смс
  }
  //if (Sim800l.available()) Serial.println("OK");
  delay(30);
  //Serial.println("Check");
}


//-----------------------------------------------------------------------------------------------------
//надо проверить код
//
//#include <LiquidCrystal.h>
//
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Объект для работы с LCD дисплеем
//const int analogPin = A0; // Пин, к которому подключено напряжение
//const int maxVoltage = 5; // Максимальное напряжение питания
//
//void setup() {
//  lcd.begin(16, 2); // Инициализация LCD дисплея
//}
//
//void loop() {
//  int sensorValue = analogRead(analogPin); // Считываем значение с аналогового пина
//  float voltage = sensorValue * maxVoltage / 1023.0; // Преобразуем значение в напряжение
//  int batteryPercent = map(voltage, 0, maxVoltage, 0, 100); // Конвертируем напряжение в процент заряда
//
//  lcd.clear(); // Очищаем экран перед выводом новых данных
//  lcd.setCursor(0, 0); // Устанавливаем курсор на первую строку и первый столбец
//  lcd.print("Battery Level:"); // Выводим текст на LCD дисплей
//  lcd.setCursor(0, 1); // Устанавливаем курсор на вторую строку и первый столбец
//  lcd.print(batteryPercent); // Выводим процент заряда
//  lcd.print("%"); // Выводим знак процента
//
//  delay(1000); // Ждем 1 секунду перед повторным чтением
//}
