/*
 * 2 в 1
 * Система защиты от протечек, и сигнализация утечки природного газа с уведомлениями по смс
 * система будет мониторить зоны где установлены датчики на случай протечки воды,
 * так же датчики газа будут установлены в зонах не доступынх глазу и носу,
 * в качестве индикации будет выступать дисплей 16х2
 * Датчики протечки подключаем к пинам 2-7, датчики газа к А1-А3,
 * 2 кнопки 11 и 12 пины, пищалка 13, GSM модуль пины 7, 8, дисплей к пинам 5 и 6.
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
#define test_alarm_sec      1                                 // пищим столько раз (секунд) для теста сигнализации
#define limit_value         150                               // пороговое значение с датчиков газа для включения тревоги

//имена зон конотроля протечки
#define kitchen             2
#define water_filter        3
#define bath                4
#define washer1             5
#define washer2             6
#define sewerage            7

//для запроса заряда батереи
#define OK                  1
#define NOTOK               2
#define TIMEOUT             3

Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
LiquidCrystal_I2C lcd(0x27, 16, 2);                           // инициализируем переменную lcd
EncButton<EB_TICK, test_pin> test_butt;                       // инициализируем кнопку теста
EncButton<EB_TICK, alarm_off_pin> alarm_off_butt;             // инициализируем кнопку отключения тревоги


// переменные для хранения показаний датчиков газа
int gas_sensor1;
int gas_sensor2;
int gas_sensor3;


bool sent = false;                                            // хранит успешность отсылки СМС
//bool sent2 = false;
bool buzzer = false;                                          // состояние сигнализации
bool water_alarm = false;                                     // состояние протечки воды
bool gas_alarm = false;                                       // состояние утечки газа

// переменные счетчиков
unsigned long last_time_gas = 0;
unsigned long last_time_water = 0;
unsigned long last_time_pik = 0;
unsigned long last_batt_val_request = 0;

// запрос статуса аккумулятора
String bat;


void clear_string(int row) {                                  // функция очищает строку дисплея
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void alarm_test() {                                           // тест сигнализации
  Serial.println("alarm test");                               //////////////////////////
  //lcd.backlight();                                            // включаем подсветку дисплея
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print("   Alarm test");
  buzz_alarm(test_alarm_sec);                                 // включаем пищалку на тестовое количесво секунд
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print(" Sending SMS...");
  sent = false;
  sent = Sim800l.sendSms(tel_number, "Alarm test, it's OK");  // отсылаем тестовое СМС
  //sent2 = Sim800l.sendSms(tel_number2, "Alarm test, it's OK");
  clear_string(1);
  if (sent) {                                                 // если модуль отослал СМС то пишем на экране результат
    lcd.setCursor(0, 1);
    lcd.print("   SMS SENT");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("  SMS ERROR");
  }
  delay(1000);
  //lcd.noBacklight();                                           // отключаем подсветку дисплея
}

void SendSMS(String SMStext_zone) {                            // отсылает СМС с названием зоны тревоги
  sent = Sim800l.sendSms(tel_number, "Alarm! Zone: " + SMStext_zone);
  //sent2 = Sim800l.sendSms(tel_number2, SMStext + SMStext_zone);
  Serial.println("alarm message sent");                        //////////////////////////
}

void buzz_pik() {
  if (buzzer) {
    if (millis() - last_time_pik > 20000) {                    // если была тревога пищим каждые 20 секунд
      digitalWrite(buzz_pin, HIGH);
      delay(100);
      digitalWrite(buzz_pin, LOW);
      delay(100);
      last_time_pik = millis();
    }
  }
}

void buzz_alarm(int alarm_time) {                              // звуковая сигнализация, на вход подаем время пищания
  int count = alarm_time;                                      // пищим столько раз (секунд)
  while (count > 0) {
    digitalWrite(buzz_pin, HIGH);
    delay(500);
    digitalWrite(buzz_pin, LOW);
    delay(500);
    count -= 1;
  }
}

void alarm_water(String display_zone, String sms_zone) {       // если найдена протечка (зоны на протечку больше не сканируются)
  //lcd.backlight();                                             // включаем подсветку дисплея при тревоге
  water_alarm = true;                                          // переводим в режим тревоги
  buzzer = true;
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print(display_zone);
  Serial.println("Water alarm!");                              //////////////////////////
  Serial.println(display_zone);                                //////////////////////////
  Serial.println(sms_zone);                                    //////////////////////////
  buzz_alarm(alarm_sec);                                       // пищим сигналкой
  SendSMS(sms_zone);                                           // отправляем СМС
}

void alarm_gas() {                                             // если найдена утечка газа
  gas_alarm = true;                                          // переводим в режим тревоги (рисуется символ на 1й строке дисплея)
  buzzer = true;
  //lcd.backlight();                                             // включаем подсветку дисплея при тревоге
  Serial.println("Gas alarm!");                                //////////////////////////
  buzz_alarm(alarm_sec);                                       // пищим сигналкой
  SendSMS("GAS!");                                             // отправляем СМС
}

void gas_view() {                                              // выводит показания датчиков на экран и в порт
  Serial.print("sensor A1 - ");                                //////////////////////////
  Serial.println(gas_sensor1);                                 //////////////////////////
  Serial.print("sensor A2 - ");                                //////////////////////////
  Serial.println(gas_sensor2);                                 //////////////////////////
  Serial.print("sensor A3 - ");                                //////////////////////////
  Serial.println(gas_sensor3);                                 //////////////////////////
  clear_string(0);
  lcd.setCursor(0, 0);
  lcd.print(gas_sensor1);
  lcd.setCursor(6, 0);
  lcd.print(gas_sensor2);
  lcd.setCursor(12, 0);
  lcd.print(gas_sensor3);
}

void gas_scan() {                                              // получаем значения с датчиков газа
  gas_sensor1 = analogRead(A1);
  gas_sensor2 = analogRead(A2);
  gas_sensor3 = analogRead(A3);
}

void sim800_check() {                                          // проверяем работу СМС модуля (работает до инициализации)
  if (SIM800.available()) {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L       OK");
    Sim800l.begin();                                           // Инициализация модема
  } else {
    lcd.setCursor(0, 1);
    lcd.print("SIM800L    ERROR");
  }
}

void sensors_check() {                                         // проверяем работу датчиков
  clear_string(1);
  lcd.setCursor(0, 1);
  lcd.print("Heating sensors");
  delay(5000);                                                 // время прогрева датчиков 
  clear_string(1);
  if (gas_sensor1 > 15 && gas_sensor2 > 15 && gas_sensor3 && 15) {  // при показаниях ниже 6 будут считаться за
    lcd.setCursor(0, 1);                                            // не работающий датчик
    lcd.print("Sensors       OK");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Sensors    ERROR");
  }
  delay(2000);
  clear_string(1);
}

void buzzer_off() {                                            // отключает сигнализацию
  if (buzzer)
    Serial.println("buzzer off");                              //////////////////////////
    buzzer = false;
}

void alarm_off() {                                             // отключает сигнализацию и режим тревоги
  if (gas_alarm || water_alarm || buzzer) {
    Serial.println("alarm off");                               //////////////////////////
    buzzer = false;
    gas_alarm = false;
    water_alarm = false;
    clear_string(1);
    lcd.setCursor(0, 1);
    lcd.print("   Alarm OFF");
    delay(3000);
    clear_string(1);
    //lcd.noBacklight();                                         // отключает подсветку дисплея
  }
}


/*******************************Запрос уровня заряда аккумулятора**********************************************/

void batt_val_request() {                                       // АТ команда с запросом заряда аккумулятора
  String cmd = Sim800l.sendATCommand("AT+CBC", true);
  String txt = cmd.substring(cmd.indexOf(",") + 1, cmd.length());
  bat = txt.toInt();
  Serial.println("send at");                                    //////////////////////////
  Serial.print(bat);                                            //////////////////////////
  Serial.println("%");                                          //////////////////////////
  lcd.setCursor(13, 1); 
  lcd.print(bat);
  lcd.print("%");
}

/************************************Основной цикл программы***************************************************/

void setup() {
  lcd.init();                                                   // инициализируем дисплей
  lcd.backlight();                                              // включение подсветки
  Serial.begin(9600);                                           // Скорость обмена данными с компьютером
  Serial.println("Start!");                                     //////////////////////////
  SIM800.begin(9600);                                           // Скорость обмена данными с модемом
  SIM800.println("AT");
  Serial.println("OK");                                         //////////////////////////
  pinMode(buzz_pin, OUTPUT);                                    // пин подключения сигнализации настраиваем на выход
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Checking...");
  sim800_check();                                               // проверка связя с модулем GSM
  delay(2000);
  sensors_check();                                              // проверяем работу датчиков
  lcd.clear();                                                  // очищаем дисплей перед стартом
  //lcd.noBacklight();                                          // отключаем подсветку дисплея чтоб зря не светил
}

void loop() {
  if (gas_alarm) {
    lcd.setCursor(15, 0);
    lcd.print(char(255));                                  // если была тревога по газу, то рисуем символ в конце 1й строки  
  }
  test_butt.tick();                                             // опрос кнопки
  if (test_butt.hasClicks(1)) {
    alarm_test();
  }
  alarm_off_butt.tick();
  if (alarm_off_butt.hasClicks(1)) {                            // одиночное нажатие отключает сигнализацию
    buzzer_off();                                               // мониторинг зоны отключен
  } else if (alarm_off_butt.hasClicks(2)) {                     // двойное нажатие сбрасывает режим тревоги
    alarm_off();                                                // включает мониторинг зон
  }
  buzz_pik();                                                   // издает короткие звуковые сигналы если была тревога
  if (!water_alarm && !gas_alarm && millis() - last_batt_val_request > 5000) {
    batt_val_request();                                         // если нет тревоги, то выводим на экран процент заряда батареи
    last_batt_val_request = millis();
  }
  if (millis() - last_time_gas > 5000) {
    Serial.println("scan GAS");                                 //////////////////////////
    gas_scan();
    gas_view();
    if (!gas_alarm) {                  // если была обнаружена утечка мониторинг продолжается, но тревога больше не включается
      if (gas_sensor1 > limit_value || gas_sensor2 > limit_value || gas_sensor3 > limit_value) {
        alarm_gas();
        Serial.println("gas alarm");                            //////////////////////////
      }
    }
    Serial.println("end scan GAS");                             //////////////////////////
    last_time_gas = millis();
  }
  if (millis() - last_time_water > 1000) {                      // проверка зон на проткчку каждую секунду
    Serial.println("Scan water");                               //////////////////////////
    if (!water_alarm) {                                         // при обнаружении протечки блок проверки зон пропускается
      lcd.setCursor(0, 1);
      lcd.print("  Water OK");
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
      Serial.println("Water alarm");                             //////////////////////////
    }
    Serial.println("end scan water");                            //////////////////////////
    last_time_water = millis();
  }
}
