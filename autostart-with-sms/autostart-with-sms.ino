#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>
#include <DHT.h>

#define BTN_PIN 2                             // кнопка остановки двигателя
#define pin_ACC 4                             // первое положение ключа
#define pin_ENGINE 5                          // зажигание
#define pin_START 6                           // стартер
#define pin_break                             // ручной тормоз
#define pin_neutral                           // коробка передач, нейтраль
#define temp_sensor A0                        // датчик температуры
#define tel "+375295689321"                   // реагируем на смс только с этого номера
#define start_ok 12                           // на этот пин приходит сигнал запуска двигателя
#define engine_start_try 3                    // попыток пуска двигателя

EncButton<EB_TICK, BTN_PIN> enc;
Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
DHT dht(temp_sensor, DHT11);                  // иниациалзация датчика температуры

String textSms, numberSms;                    // текст смс и номер абонента
volatile bool isstarted; ////////////////////////
unsigned long last_time;
float temp, time_value;
String engine_status;                         // статус двигателя: switched-off - выключен
                                              // engine-running - запущен
                                              // starting-failed - запуск не удался

void relay_off() {
  // реле управляется низким уровнем, при включении МК подаем высокий сигнал на пины
  digitalWrite(pin_ACC, HIGH);
  digitalWrite(pin_ENGINE, HIGH);
  digitalWrite(pin_START, HIGH);
  Serial.println("relay off");
}

String engine_off() {
  // глушим двигатель
  digitalWrite(pin_ENGINE, HIGH);
  delay(50);
  digitalWrite(pin_ACC, HIGH);
  return "switched off";
}

float get_time_val() {
  //измеряет температуру, возвращает время вращения стартера
  temp = dht.readTemperature();
  Serial.print("Temerature ");
  Serial.println(temp);
  if (temp >= 5) {                  // температуры выше +5 - время 1.5 сек
    return 1.5;
  } else if (temp < -10) {          // температура меньше -10 - время 2.5 сек
    return 2.5;
  } else {                          // температуры от -10 до +4 - время 2 сек
    return 2.0;
  }
}

String engine_start(float time_val) {
  if (engine_status != "engine-running") {
    int count = 0;
    Serial.println(time_val);
    digitalWrite(pin_ACC, LOW);
    Serial.println("ACC");
    delay(500);
    digitalWrite(pin_ENGINE, LOW);
    Serial.println("ENGINE");
    while (digitalRead(start_ok) != HIGH && count != engine_start_try) {
      delay(5000);
      digitalWrite(pin_START, LOW);
      delay(time_val*1000);
      digitalWrite(pin_START, HIGH);
      Serial.println("trying to start");
      count++;
    }
    delay(3000);
    if (digitalRead(start_ok) == HIGH){
      Serial.println("engine running");
      return "engine running";
    } else {
      Serial.println("starting failed");
      engine_off();
      return "starting failed";
    }
  }
}

void setup() {
  Serial.begin(9600);                         // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  SIM800.println("AT");
  Sim800l.begin();                            // Инициализация модема
  dht.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(pin_ACC, OUTPUT);
  pinMode(pin_ENGINE, OUTPUT);
  pinMode(pin_START, OUTPUT);
  relay_off();                                // реле управляются низким уровнем, подаем на них высокий, состояние - отключены
  Sim800l.delAllSms();
  Serial.println("All sms deleted");
  attachInterrupt(0, relay_off, FALLING); ////////////////////
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
          time_value = get_time_val();
          engine_status = engine_start(time_value);
          Sim800l.sendSms(tel, engine_status);
        } else if (textSms.indexOf("\nSTOP\r\n")!=-1) {
          engine_status = engine_off();
          Sim800l.sendSms(tel, engine_status);
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
//  enc.tick(); // удалить строки, попробовать прерывания
//  if (enc.hasClicks()) {
//    engine_off();
//    Serial.println("engine off");
//  }
}
