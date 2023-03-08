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
#define buzz_pin            13                                // пин сигналки
#define test_pin            11                                // кнопка теста
#define alarm_off_pin       12                                // кнопка отключения тревоги
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
EncButton<EB_TICK, test_pin> test_butt;                       // инициализируем кнопку теста
EncButton<EB_TICK, alarm_off_pin> alarm_off_butt;             // инициализируем кнопку отключения тревоги


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

void alarm_test() {                                               // тест сигнализации
  Serial.println("alarm test");
  //lcd.backlight();
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print("   Alarm test");
  buzz_alarm(test_alarm_sec);
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print(" Sending SMS...");
  sent = Sim800l.sendSms(tel_number, "Alarm test, it's OK");
  //sent2 = Sim800l.sendSms(tel_number2, "Alarm test, it's OK");
  clear_string(1);
  if (sent) {
    lcd.setCursor(0, 1);
    lcd.print("    SMS OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("  SMS ERROR");
  }
  delay(1000);
  //lcd.noBacklight();
}

void SendSMS(String SMStext_zone) {
  sent = Sim800l.sendSms(tel_number, "Alarm! Zone: " + SMStext_zone);
  //sent2 = Sim800l.sendSms(tel_number2, SMStext + SMStext_zone);
  Serial.println("alarm message sent");
}

void buzz_pik() {
  if (millis() - last_time_pik > 10000) {                      // если была тревога пищим каждые сколько-то секунд
    if (buzzer) {
      digitalWrite(buzz_pin, HIGH);
      delay(100);
      digitalWrite(buzz_pin, LOW);
      delay(100);
      last_time_pik = millis();
    }
  }
}

void buzz_alarm(int alarm_time) {                               // звуковая сигнализация, на вход подаем время пищания
  int count = alarm_time;                                       // пищим столько раз (секунд)
  while (count > 0) {
    digitalWrite(buzz_pin, HIGH);
    delay(500);
    digitalWrite(buzz_pin, LOW);
    delay(500);
    count -= 1;
  }
}

void alarm_water(String display_zone, String sms_zone) {
  //lcd.backlight();                                              // включаем подсветку дисплея при тревоге
  water_alarm = true;
  buzzer = true;
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print(display_zone);
  Serial.println("Water alarm!");
  Serial.println(display_zone);
  Serial.println(sms_zone);
  buzz_alarm(alarm_sec);
  SendSMS(sms_zone);
}

void alarm_gas() {
  gas_alarm = true;
  buzzer = true;
  //lcd.backlight();                                         // включаем подсветку дисплея при тревоге
  Serial.println("Gas alarm!");
  buzz_alarm(alarm_sec);
  SendSMS("GAS!");
}

void gas_view() {                                               // выводит показания датчиков на экран и в порт
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

void sim800_check() {                                           // проверяем работу СМС модуля (работает до инициализации)
  if (SIM800.available()) {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L       OK");
    Sim800l.begin();                                            // Инициализация модема
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L    ERROR");
  }
}

void sensors_check() {                                          // проверяем работу датчиков
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print("Heating sensors");
  delay(5000);
  clear_string(1);
  if (analogRead(A1) < 5 && analogRead(A2) < 5 && analogRead(A3) < 5) {       // пересмотреть значение
    lcd.setCursor(0, 1);
    lcd.print("Sensors       OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Sensors    ERROR");
  }
  delay(2000);
  clear_string(1);
}

void buzzer_off() {
  Serial.println("buzzer off");
  buzzer = false;
}

void alarm_off() {
  buzzer = false;
  gas_alarm = false;
  water_alarm = false;
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print("   Alarm OFF");
  delay(3000);
  clear_string(1);
  //lcd.noBacklight();
}

//float batt_val_request() {
//  
//}

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
  //attachInterrupt(0, alarm_test, FALLING);                      // настраиваем прерывание на пине
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Checking...");
  sim800_check();
  delay(2000);
  sensors_check();                                              // проверяем работу датчиков
  //delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("     Start");
  delay(2000);
  lcd.clear();
  //lcd.noBacklight();                                          // отключаем подсветку дисплея чтоб зря не светил
}

void loop() {
  if (gas_alarm) {
    lcd.setCursor(15, 1);
    lcd.print(char(255));
  }
  test_butt.tick();                                                   // опрос кнопки
  if (test_butt.hasClicks()) {
    alarm_test();
  }
  alarm_off_butt.tick();
  if (alarm_off_butt.hasClicks()) {
    buzzer_off();
  } else if (alarm_off_butt.hasClicks(2)) {
    alarm_off();
  }
  buzz_pik();                                                   // издает короткие звуковые сигналы если была тревога
  if (millis() - last_time_gas > 5000) {
    Serial.println("scan GAS");
    gas_scan();
    gas_view();
    if (!gas_alarm) {                     // если была обнаружена утечка мониторинг продолжается, но тревога не включается
      if (gas_sensor1 > limit_value || gas_sensor2 > limit_value || gas_sensor3 > limit_value) {
        alarm_gas();
        Serial.println("gas alarm");
      }
    }
    Serial.println("end scan GAS");
    last_time_gas = millis();
  }
  if (millis() - last_time_water > 1000) {
    Serial.println("Scan water");
    if (!water_alarm) {                                  // при обнаружении протечки блок проверки зон пропускается
      lcd.setCursor(0, 1);
      lcd.print("    Water OK");
      if (digitalRead(kitchen) == HIGH) {
        alarm_water("Kitchen", "kitchen");
      }
      if (digitalRead(water_filter) == HIGH) {
        alarm_water("Water filter", "kitchen, water filter");
      }
      if (digitalRead(bath) == HIGH) {
        alarm_water("Bath", "bathroom, under the bath");
      }
      if (digitalRead(washer1) == HIGH || digitalRead(washer2) == HIGH) {
        alarm_water("Washing machine", "washing machine");
      }
      if (digitalRead(sewerage) == HIGH) {
        alarm_water("B*room sewerage", "sewerage, bathroom, behind the toilet");
      }
    } else {
      Serial.println("Water alarm");
    }
    Serial.println("end scan water");
    last_time_water = millis();
  }
}
