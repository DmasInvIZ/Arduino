/*
 * Скетч представляет собой эхо-смс.
 * Шлет тестово сообщение на указанный номер,
 * а в цикле loop ждет смс, и шлет ответ на него на тот же номер
 * с текстом содержащимся в переменной.
 */

#include <SoftwareSerial.h>
#include <Sim800l.h>

Sim800l Sim800l;
bool sending;

SoftwareSerial SIM800(8, 9);        // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)

String sms_text = "This is test message!";  // такое сообщение
String tel_number = "+375295689321";        // на этот номер

String numberSms, textSms;
const char ans = "OK";


void setup() {
  Serial.begin(9600);               // Скорость обмена данными с компьютером
  Serial.println("Start!");
  SIM800.begin(9600);               // Скорость обмена данными с модемом
  SIM800.println("AT");

  Sim800l.begin();                                                       // инициализируем библиотеку
  sending = Sim800l.sendSms(tel_number, sms_text);   // шлем сообщение
  Serial.println("Message sent");
}

void loop() {
  textSms=Sim800l.readSms(1);                                            // читаем смс
  if (textSms.indexOf("OK")!=-1) {                                       // убеждаемся что пришел ответ от модуля о выполнении команды
    numberSms=Sim800l.getNumberSms(1);                                   // получаем номер абонента приславшего смс
    Serial.println(numberSms);                                           // пишем в консоль номер абонента приславшего смс
    Sim800l.sendSms(numberSms, textSms);                              // когда получаем смс отправляем ответ
    Sim800l.delAllSms();                                                 // удаляет все сообщения
  }
  Serial.println("iter");                                                // выводит при каждой итерации цикла loop
}
