/*
  Система сигнализации протечек.
  Имеем 4 зоны контроля и 7 датчиков
  Кухня, под ванной, 3 под стиральной машиной, 2 в стояке с коммуникациями
  При срабатывании любого датчика включается сигнализация на 30 сек.
  В перспективе подключение к SIM800L для отправки СМС на телефон
  - Подключение датчиков: пины 2-8
  - Для сигнализации временно используем встроенный светодиод на пине 13
*/


//bool kitchen = digitalRead(2);
//bool bath = digitalRead(3);
//bool washer1 = digitalRead(4);
//bool washer2 = digitalRead(5);
//bool sewerage1 = digitalRead(6);
//bool sewerage2 = digitalRead(7);

#define kitchen digitalRead(2)
#define bath digitalRead(3)
#define washer1 digitalRead(4)
#define washer2 digitalRead(5)
#define sewerage1 digitalRead(6)
#define sewerage2 digitalRead(7)


unsigned long last_time;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  if (millis() - last_time > 1000) {
    if (kitchen || bath || washer1 || washer2 || sewerage1 || sewerage2 == HIGH) {
      alarm_on();
    }
    else {
      Serial.println("Check");
      Serial.println(round(millis() / 1000));
    }
  // delay(1000);
  
    last_time = millis();
  }

}


void alarm_on() {
  
  if (millis() - last_time > 5000) {
    Serial.println("ALARM");
    led_blink();
    last_time = millis();
  }
  
}

void led_blink() {
  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);
  delay(100);
}
