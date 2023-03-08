/*
 * 2 в 1
 * Система защиты от протечек, и сигнализация утечки природного газа с уведомлениями по смс
 * первая почти готова, вторая будет следить с помощью нескольких датчиков за зонами недоступными глазу
 * в качестве индикации будет выступать дисплей 16х2
 */

#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>
#include <LiquidCrystal_I2C.h>

// пины подключения
#define buzz_pin 13                                           // пин сигналки   
// смс 
#define tel_number          "+375295689321"                   // номер телефона для отправки
#define tel_number2         "+375257769952"                   // 2 номер телефона для отправки
// сигнализация
#define alarm_sec           2                                 // пищим столько раз (секунд)
#define test_alarm_sec      2                                 // пищим столько раз (секунд) для теста сигнализации
#define limit_value         150                               // пороговое значение с датчиков газа для включения тревоги


Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
LiquidCrystal_I2C lcd(0x27, 16, 2);                           // инициализируем переменную lcd


//имена зон конотроля
int kitchen = 10;
int water_filter = 3;
int bath = 4;
int washer1 = 5;
int washer2 = 6;
int sewerage = 7;
// датчики газа
int gas_sensor1;
int gas_sensor2;
int gas_sensor3;

bool sent = false;

//bool sent2 = false;
bool buzzer = false;
volatile bool water_alarm = false;
volatile bool gas_alarm = false;
unsigned long last_time_gas = 0;
unsigned long last_time_water = 0;
unsigned long last_time_pik = 0;

void clear_string(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

//void alarm_test() {
//  Serial.println("alarm test");
//  int test_counter = test_alarm_sec;
//  while (test_counter > 0) {
//    digitalWrite(led_kitchen, HIGH);
//    digitalWrite(led_water_filter, HIGH);
//    digitalWrite(led_bath, HIGH);
//    digitalWrite(led_washer, HIGH);
//    digitalWrite(led_sewerage, HIGH);
//    digitalWrite(buzz_pin, HIGH);
//    delay(500);
//    digitalWrite(led_kitchen, LOW);
//    digitalWrite(led_water_filter, LOW);
//    digitalWrite(led_bath, LOW);
//    digitalWrite(led_washer, LOW);
//    digitalWrite(led_sewerage, LOW);
//    digitalWrite(buzz_pin, LOW);
//    delay(500);
//    test_counter -= 1;
//  }
//  Sim800l.sendSms(tel_number, "Test SMS, module OK");
//  Serial.println("send sms");
//}

void SendSMS(String SMStext_zone) {
  sent = Sim800l.sendSms(tel_number, "Alarm! Zone: " + SMStext_zone);
  //sent2 = Sim800l.sendSms(tel_number2, SMStext + SMStext_zone);
  Serial.println("alarm message sent");
}

void buzz_pik() {
  if (millis() - last_time_pik > 10000) {                               // пищим каждые сколько-то секунд
    if (buzzer) {
      digitalWrite(buzz_pin, HIGH);
      delay(100);
      digitalWrite(buzz_pin, LOW);
      delay(100);
      last_time_pik = millis();
    }
  }
}

void alarm_water(String display_zone, String sms_zone) {
  water_alarm = true;
  buzzer = true;
  //lcd.backlight();                                      // включаем подсветку дисплея при тревоге
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print("W "+display_zone);
  Serial.println("Alarm!");
  Serial.println(display_zone);
  Serial.println(sms_zone);
  int counter = alarm_sec;                                 // пищим столько раз (секунд)
  while (counter > 0) {
    digitalWrite(buzz_pin, HIGH);
    delay(500);
    digitalWrite(buzz_pin, LOW);
    delay(500);
    counter -= 1;
  }
  SendSMS(sms_zone);
}

/************************************Основной цикл программы***************************************************/

void setup() {
  lcd.init();                                                   // инициализируем дисплей
  lcd.backlight();                                              // включение подсветки
  Serial.begin(9600);                                           // Скорость обмена данными с компьютером
  Serial.println("Start!");
  SIM800.begin(9600);                                           // Скорость обмена данными с модемом
  SIM800.println("AT");
  Serial.println("OK");
  pinMode(buzz_pin, OUTPUT);
  //attachInterrupt(0, alarm_test, FALLING);                    // настраиваем прерывание на пине
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking...");
  if (SIM800.available()) {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L       OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L    ERROR");
  }
  Sim800l.begin();                                              // Инициализация модема
  delay(2000);
  clear_string(1);
  if (analogRead(A1) < 5 && analogRead(A2) < 5 && analogRead(A3) < 5) {     // проверяем работу датчиков
    lcd.setCursor(0, 1);
    lcd.print("Sensors       OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Sensors    ERROR");
  }
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  OK");
  delay(1000);
  //lcd.noBacklight();                                            // отключаем подсветку дисплея чтоб зря не светил
}

void gas_view() {
  Serial.print("sensor A1 - ");
  Serial.println(gas_sensor1);
  Serial.print("sensor A2 - ");
  Serial.println(gas_sensor2);
  Serial.print("sensor A3 - ");
  Serial.println(gas_sensor3);
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print(gas_sensor1);
  lcd.setCursor(6, 0);
  lcd.print(gas_sensor2);
  lcd.setCursor(12, 0);
  lcd.print(gas_sensor3);
}

void gas_scan() {
  gas_sensor1 = analogRead(A1);
  gas_sensor2 = analogRead(A2);
  gas_sensor3 = analogRead(A3);
}

void loop() {
  buzz_pik();                                                     // издает короткие звуковые сигналы если была тревога
  if (millis() - last_time_gas > 5000) {
    Serial.println("scan GAS");
    if (!gas_alarm) {                                   // при обнаружении утечки газа блок проверки пропускается
      gas_scan();
      gas_view();
      if (gas_sensor1 > limit_value || gas_sensor2 > limit_value || gas_sensor3 > limit_value) {
        // продумать сигнализацию на случай утечки газа
      }
      Serial.println("end scan GAS");
    } else {
      Serial.print("gas alarm");
    }
    last_time_gas = millis();
  }
  if (millis() - last_time_water > 1000) {
    Serial.println("Scan water");
    if (!water_alarm) {                                  // при обнаружении протечки блок проверки зон пропускается
      if (digitalRead(kitchen) == HIGH) {
        alarm_water("Kitchen", "kitchen");
      }
      if (digitalRead(water_filter) == HIGH) {
        alarm_water("Water filter", "kitchen, water filter");
      }
      if (digitalRead(bath) == HIGH) {
        alarm_water("Bath", "bathroom, under bath");
      }
      if (digitalRead(washer1) == HIGH || digitalRead(washer2) == HIGH) {
        alarm_water("Washingmachine", "washing machine");
      }
      if (digitalRead(sewerage) == HIGH) {
        alarm_water("Sewerage", "sewerage, bathroom, behind the toilet");
      }
    } else {
      Serial.println("Water alarm");
    }
    Serial.println("end scan water");
    last_time_water = millis();
  }
}
