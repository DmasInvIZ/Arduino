#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <EncButton.h>


#define BTN_PIN 3                             // кнопка остановки двигателя
#define pin_ENGINE 5                          // зажигание
#define pin_START 6                           // стартер
//#define pin_break                             // ручной тормоз
//#define pin_neutral                           // коробка передач, нейтраль
#define tel "+375295689321"                   // реагируем на смс только с этого номера
#define start_ok_pin 12                       // на этот пин приходит сигнал запуска двигателя

EncButton<EB_TICK, BTN_PIN> enc;
Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);                  // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

String textSms, numberSms;                    // текст смс и номер абонента
unsigned long last_time;
String engine_status;                         // статус двигателя: switched-off - выключен
                                              //                   engine-running - запущен
                                              //                   starting-failed - запуск не удался

void relay_off() {
  // реле управляется низким уровнем, при включении МК подаем высокий сигнал на пины
  digitalWrite(pin_ENGINE, HIGH);
  digitalWrite(pin_START, HIGH);
  Serial.println("relay off");
}

//включает зажигание
void ignition_on() {
  digitalWrite(pin_ENGINE, LOW);
}

//выключает зажигание
void ignition_off() {
  digitalWrite(pin_ENGINE, HIGH);
}

//включает стартер
void starter_on() {
  digitalWrite(pin_START, LOW);
}

//выключает стартер
void starter_off() {
  digitalWrite(pin_START, HIGH);
}

// Функция выключения двигателя
// ----------------------------------
String engine_off() {
  Serial.println("[REQUEST] Engine OFF sequence initiated...");

  if (engine_status != "engine-running") {
    Serial.println("[INFO] Engine already OFF. No action taken.");
    return engine_status = "switched-off";
  }

  starter_off();   // на всякий случай
  ignition_off();

  Serial.println("[ACTION] Ignition OFF. Waiting for engine to stop...");

  unsigned long timeout = millis();
  while (millis() - timeout < 3000) { // ждём до 3 секунд подтверждения остановки
    if (digitalRead(start_ok_pin) == LOW) {
      engine_status = "switched-off";
      Serial.println("[SUCCESS] Engine stopped successfully.");
      return engine_status;
    }
    delay(100);
  }

  engine_status = "stop-failed";
  Serial.println("[ERROR] Engine stop timeout reached, engine still running!");
  return engine_status;
}

//проверяет запущен ли двигатель
bool status_checking(){
  return digitalRead(start_ok_pin);
}

//запускает двигатель
String engine_start() {
  // Если двигатель уже работает — выходим сразу
  bool engine = status_checking();
  if (engine) {
    Serial.println("[INFO] Engine already running, no action needed.");
    return engine_status;
  }

  Serial.println("[INFO] Starting engine sequence...");
  ignition_on();               
  Serial.println("[ACTION] Ignition ON.");
  delay(2000);                 // ждём стабилизации питания

  const int max_attempts = 3;  
  const unsigned long max_starter_time = 5000; // максимум 5 секунд кручения стартера

  for (int attempt = 1; attempt <= max_attempts; attempt++) {
    Serial.print("\n[ATTEMPT ");
    Serial.print(attempt);
    Serial.println("] Starting attempt...");

    unsigned long start_time = millis();
    starter_on();
    Serial.println("[ACTION] Starter ON.");

    bool started = false;
    while (millis() - start_time < max_starter_time) {
      if (digitalRead(start_ok_pin) == HIGH) {
        // двигатель запущен
        started = true;
        break;
      }

      // Каждую секунду выводим, сколько времени стартер крутится
      unsigned long elapsed = millis() - start_time;
      if (elapsed % 1000 < 100) {
        Serial.print("[INFO] Starter running for ");
        Serial.print(elapsed / 1000.0, 1);
        Serial.println("s...");
      }
      delay(100);
    }

    starter_off();
    unsigned long total_time = millis() - start_time;
    Serial.print("[ACTION] Starter OFF. Duration: ");
    Serial.print(total_time / 1000.0, 2);
    Serial.println("s");

    if (started) {
      engine_status = "engine-running";
      Serial.println("[SUCCESS] Engine started successfully!");
      return engine_status;
    } else {
      Serial.println("[WARN] Engine did not start on this attempt.");
      delay(3000); // пауза между попытками
    }
  }

  ignition_off();
  engine_status = "starting-failed";
  Serial.println("[ERROR] Engine failed to start after 3 attempts. Ignition OFF.");
  return engine_status;
}


void setup() {
  Serial.begin(9600);                         // Инициализация последовательной связи с Arduino и Arduino IDE (Serial Monitor)
  Serial.println("Loading...");
  SIM800.begin(9600);                         // Инициализация последовательной связи с Arduino и SIM800L
  SIM800.println("AT");
  Sim800l.begin();                            // Инициализация модема
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(start_ok_pin, INPUT);               // ПОДТЯНУТЬ ПИН К ЗЕМЛЕ
  pinMode(pin_ENGINE, OUTPUT);
  pinMode(pin_START, OUTPUT);
  relay_off();                                // реле управляются низким уровнем, подаем на них высокий, состояние - отключены
  Sim800l.delAllSms();
  Serial.println("All sms deleted");
  Serial.println("Ready");

  //engine_start(); //for testing
}

void loop() {
  // обновляем состояние энкодера/кнопки
  enc.tick();

  // если кнопка нажата — пробуем выключить двигатель
  if (enc.press()) {
    Serial.println("[INPUT] Engine OFF button pressed!");
    engine_off();
  }
  
  if (millis() - last_time > 10000) {
    Serial.println("check sms");
    textSms=Sim800l.readSms(1);
    if (textSms.indexOf("OK")!=-1) {
      numberSms=Sim800l.getNumberSms(1);
      Serial.println(numberSms);
      if (numberSms == tel) {
        Serial.println("this number!");
        textSms.toUpperCase();
        if (textSms.indexOf("\nSTART\r\n")!=-1) {
          engine_status = engine_start();
          delay(5000);
          Sim800l.sendSms(tel, engine_status);
        } else if (textSms.indexOf("\nSTOP\r\n")!=-1) {
          engine_status = engine_off();
          delay(5000);
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
}
