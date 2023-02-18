
#include <SoftwareSerial.h>
#include <Sim800l.h>

Sim800l Sim800l;
bool sending;

String text;
uint8_t index;

SoftwareSerial SIM800(8, 9);        // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

void setup() {
  Serial.begin(9600);               // Скорость обмена данными с компьютером
  Serial.println("Start!");
  SIM800.begin(9600);               // Скорость обмена данными с модемом
  SIM800.println("AT");

//  Sim800l.begin(); // initializate the library. 
//  sending = Sim800l.sendSms("+375295689321", "Text");
//  Serial.println(sending);

  index=1; // first position in the prefered memory storage. 
  text=Sim800l.readSms(index);    
  Serial.println(text);
}
void loop() {
  if (SIM800.available())           // Ожидаем прихода данных (ответа) от модема...
    Serial.write(SIM800.read());    // ...и выводим их в Serial
  if (Serial.available())           // Ожидаем команды по Serial...
    SIM800.write(Serial.read());    // ...и отправляем полученную команду модему
}
