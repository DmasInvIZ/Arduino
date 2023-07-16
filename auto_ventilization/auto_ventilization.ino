#include <DHT.h>
#include <EncButton.h>

#define temp_sensor_pin 2
#define relay_pin 4
#define BTN_PIN 8
#define temp_val 29
#define manual_on 7


EncButton<EB_TICK, BTN_PIN> butt;                 // инициализация кнопки
DHT dht(temp_sensor_pin, DHT11);                  // иниациалзация датчика температуры

unsigned long last_time_check;
bool activate = false;                            // статус реле
float temp;                                       // показания температуры

void fan_check() {
  delay(2000);
  digitalWrite(relay_pin, HIGH);
}


void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(relay_pin, OUTPUT);
  fan_check();
  Serial.println("start");
}

void loop() {
  if (digitalRead(manual_on) == HIGH) {
    activate = true;
  } else {
    activate = false;
  }
  butt.tick();
  if (butt.hasClicks()) {
    digitalWrite(relay_pin, HIGH);
    Serial.println("delay");
    delay(10000);
  }
  if (activate == true) {
    Serial.println("on");
    digitalWrite(relay_pin, LOW);
    } else {
    Serial.println("off");
    digitalWrite(relay_pin, HIGH);
    }
  if (millis() - last_time_check > 1000) {
    Serial.println("readTemperature");
    temp = dht.readTemperature();
    Serial.println(temp);
    if (temp > temp_val) {
      Serial.println("true");
      activate = true;
    } else {
      Serial.println("false");
      activate = false;
    }
    last_time_check = millis();
  }
}
