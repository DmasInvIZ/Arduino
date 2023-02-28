#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>

#define BTN_PIN 2                             // кнопка остановки двигателя
#define pin_ACC 4                             // первое положение ключа
#define pin_ENGINE 5                          // зажигание
#define pin_START 6                           // стартер
#define pin_break                             // ручной тормоз
#define pin_neutral                           // коробка передач, нейтраль
#define temp_sensor A0                        // датчик температуры
#define tel "+375295689321"                   // реагируем на смс только с этого номера
#define start_ok A1                           // на этот пин приходит сигнал запуска двигателя
#define engine_start_try 3                    // попыток прокрутки стартера

EncButton<EB_TICK, BTN_PIN> enc;
Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

String textSms, numberSms;                     // текст смс и номер абонента
volatile bool isstarted; ////////////////////////
unsigned long last_time;
float temp, time_val;
//float test_temp_val;
bool is_started;
int counter;

void relay_off() {
  // реле управляется низким уровнем, при включении МК подаем высокий сигнал на пины
  digitalWrite(pin_ACC, HIGH);
  digitalWrite(pin_ENGINE, HIGH);
  digitalWrite(pin_START, HIGH);
}

void engine_off() {
  // глушим двигатель
  digitalWrite(pin_ENGINE, HIGH);
  delay(50);
  digitalWrite(pin_ACC, HIGH);
}

float get_time_val() {
  //измеряет температуру, возвращает время вращения стартера
  temp = 15;
  if (temp >= 5) {
    time_val = 1.5;
  } else if (temp < -10) {
    time_val = 2.5;
  } else {
    time_val = 2.0;
  }
  return time_val;
}

bool engine_start(float time_val) {
  if (is_started != true) {
    digitalWrite(pin_ACC, LOW);
    Serial.println("ACC");
    delay(500);
    digitalWrite(pin_ENGINE, LOW);
    Serial.println("ENGINE");
    delay(5000);
    while (analogRead(start_ok) != HIGH && counter != engine_start_try) {
      digitalWrite(pin_START, LOW);
      delay(time_val);
      digitalWrite(pin_START, HIGH);
      Serial.println("trying to start");
      counter += 1;
    }
  }
  is_started = true;
  Serial.println("engine started");
  return is_started;
}

void setup() {
  Serial.begin(9600);                         // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  SIM800.println("AT");
  Sim800l.begin();                            // Инициализация модема
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(pin_ACC, OUTPUT);
  pinMode(pin_ENGINE, OUTPUT);
  pinMode(pin_START, OUTPUT);
  relay_off();                                // реле управляются низким уровнем, подаем на них высокий, состояние - отключены
  Sim800l.delAllSms();
  Serial.println("All sms deleted");
  attachInterrupt(2, engine_off, FALLING); ////////////////////
  Serial.println("Ready");
}

void loop() {
  if (millis() - last_time > 5000) {
    Serial.println("check sms");
    textSms=Sim800l.readSms(1);
    if (textSms.indexOf("OK")!=-1) {
      numberSms=Sim800l.getNumberSms(1);
      Serial.println(numberSms);
      if (numberSms == tel) {
        Serial.println("this number!");
        textSms.toUpperCase();
        if (textSms.indexOf("\nSTART\r\n")!=-1) {
          if (isstarted != true) {
            Serial.println("start!");
            digitalWrite(pin_ACC, LOW);
            isstarted = true;
            Sim800l.delAllSms();
            Serial.println("del sms");
            return isstarted;
          } else {
            Serial.println("already started");
          }
        } else if (textSms.indexOf("\nSTOP\r\n")!=-1) {
          engine_off();
          isstarted = false;
          Serial.println("engine off");
        } else {
          Serial.println("wrong phrase");
        }
      } else {
        Serial.println("wrong number");
      }
      Sim800l.delAllSms();
      Serial.println("del sms");
    }
    last_time = millis();
    Serial.println("end check sms");
  }
  enc.tick(); // удалить строки, попробовать прерывания
  if (enc.hasClicks()) {
    engine_off();
    isstarted = false;
    Serial.println("engine off");
  }
}
