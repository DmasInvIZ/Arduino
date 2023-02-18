#include "DHT.h"
#define DHTPIN 2                           //пин подключения датчика
#define fanPin 9                           //пин подключения вентилятора (реле)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(fanPin, OUTPUT);
  dht.begin();
  fanON();                                 //
  delay(5000);                             //включить для проверки на 5 сек
  fanOFF();                                //по умолчанию выключен
}
void loop() {
  int h = dht.readHumidity();
  if (h > 90) {                           //если влажность выше 50%, вентилятор включается
    fanON();
  } else {
    fanOFF();
  }
  delay(2000);                            //замер влажности каждые 2 секунды
}

void fanON() {                            //функция включения вентилятора
  digitalWrite(fanPin, 0);
}
void fanOFF() {                           //функция отключения вентилятора
  digitalWrite(fanPin, 1);
}
