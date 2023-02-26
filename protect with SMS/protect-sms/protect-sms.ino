/*
  Система сигнализации протечек.
  Имеем 5 зон контроля и 6 датчиков
  Под мойкой в кухне, под фильтром в кухне, под ванной, 2 под стиральной машиной, 1 в стояке с коммуникациями
  При срабатывании любого датчика включается сигнализация на 20 сек.
  После того как обнаружена протечка включается пищалка, отсылается смс на указанные номера 
  и мигает соответсвутющая лампочка.
  Работа алгоритма прекращается до перезагрузки МК.

  - Подключение датчиков: пины 2-7
  - 7, 8 пины для подключения sim800l
  - 11 пин - пищалка
  - A0-A5 - для светодиодов

  В следующих версиях прошивки:
  работы программы не останавливается после обнаружения протечки
*/

#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>
#include "GyverButton.h"

#define buzz_pin 11                                           // пин сигналки   
#define BTN_PIN 10                                            // кнопка
#define SMStext "Alarm! Zone: "                               // текст смс на телефон
#define tel_number "+375295689321"                            // номер телефона для отправки
#define tel_number2 "+375257769952"                           // 2 номер телефона для отправки
#define alarm_sec 2                                           // пищим столько раз (секунд)

#define led_kitchen A0
#define led_water_filter A1
#define led_bath A2
#define led_washer A3
#define led_sewerage A4

GButton butt1(BTN_PIN);

Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

//char* SMStext;
//char* tel_number;
//char* tel_number2;


//имена зон конотроля
int kitchen = 2;
int water_filter = 3;
int bath = 4;
int washer1 = 5;
int washer2 = 6;
int sewerage = 7;

void led_blink(int zone) {
  while (true) {
    digitalWrite(zone, LOW);
    delay(300);
    digitalWrite(zone, HIGH);
    delay(300);
  }
}

void SendSMS(String SMStext_zone) {
  Sim800l.sendSms(tel_number, SMStext + SMStext_zone);
  Sim800l.sendSms(tel_number2, SMStext + SMStext_zone);
  Serial.println("alarm message sent");
}

void alarm_on(int led, String zone) {
  Serial.println("Alarm!");
  Serial.println(led);
  digitalWrite(led, HIGH);
  int counter = alarm_sec;                                       // пищим столько раз (секунд)
  while (counter > 0) {
    digitalWrite(buzz_pin, HIGH);
    delay(500);
    digitalWrite(buzz_pin, LOW);
    delay(500);
    counter -= 1;
  }
  SendSMS(zone);
  Serial.println("led blink");
  led_blink(led);                                               // включаем светодиод на панели для наглядности
}

void setup() {
  Serial.begin(9600);                         // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  Sim800l.begin();                            // Инициализация модема
  SIM800.println("AT");
  pinMode(buzz_pin, OUTPUT);
  pinMode(led_kitchen, OUTPUT);
  pinMode(led_water_filter, OUTPUT);
  pinMode(led_bath, OUTPUT);
  pinMode(led_washer, OUTPUT);
  pinMode(led_sewerage, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);             // пин кнопки с внутренней подтяжкой
  delay(1000);
  Serial.println("Ready.");
}

void loop() {
  if (digitalRead(kitchen) == HIGH) {
    alarm_on(led_kitchen, "kitchen");                                    // led заменить на пин светодиода
  }
  if (digitalRead(water_filter) == HIGH) {
    alarm_on(led_water_filter, "kitchen, water filter");
  }
  if (digitalRead(bath) == HIGH) {
    alarm_on(led_bath, "bathroom, under bath");
  }
  if (digitalRead(washer1) == HIGH || digitalRead(washer2) == HIGH) {
    alarm_on(led_washer, "washing machine");
  }
  if (digitalRead(sewerage) == HIGH) {
    alarm_on(led_sewerage, "sewerage, bathroom, behind the toilet");
  }
  butt1.tick();
  if (butt1.isSingle()) {
    Sim800l.sendSms(tel_number, "Test SMS, module OK");
    Sim800l.sendSms(tel_number2, "Test SMS, module OK");
    Serial.println("send sms");                                    // по нажатию кнопки отправляем тестовое смс
  }
  //if (Sim800l.available()) Serial.println("OK");
  delay(30);
  //Serial.println("Check");
}
