// библиотека для работы с датчиками MQ (Troyka-модуль)
#include <TroykaMQ.h>
#include <LiquidCrystal_I2C.h>
 
// имя для пина, к которому подключен датчик
#define PIN_MQ41         A1
#define PIN_MQ42         A2
#define PIN_MQ43         A3
// имя для пина, к которому подключен нагреватель датчика
#define PIN_MQ4_HEATER  13

LiquidCrystal_I2C lcd(0x27, 20, 4);
 
// создаём объект для работы с датчиком
// и передаём ему номер пина выходного сигнала и нагревателя
MQ4 sens1(PIN_MQ41, PIN_MQ4_HEATER);
MQ4 sens2(PIN_MQ42, PIN_MQ4_HEATER);
MQ4 sens3(PIN_MQ43, PIN_MQ4_HEATER);
 
void setup()
{
  // открываем последовательный порт
  Serial.begin(9600);
  // включаем нагреватель
  sens1.heaterPwrHigh();
  sens2.heaterPwrHigh();
  sens3.heaterPwrHigh();
  Serial.println("Heated sensor");
}
 
void loop()
{
  // если прошёл интервал нагрева датчика
  // и калибровка не была совершена
  if (!sens1.isCalibrated() && sens1.heatingCompleted()) {
    // выполняем калибровку датчика на чистом воздухе
    sens1.calibrate();
    sens2.calibrate();
    sens3.calibrate();
    // выводим сопротивление датчика в чистом воздухе (Ro) в serial-порт
    Serial.print("Ro = ");
    Serial.println(sens1.getRo());
    
  }
  // если прошёл интервал нагрева датчика
  // и калибровка была совершена
  if (sens1.isCalibrated() && sens1.heatingCompleted()) {
    // выводим отношения текущего сопротивление датчика
    // к сопротивлению датчика в чистом воздухе (Rs/Ro)
//    Serial.print("Ratio: ");
//    Serial.print(mq41.readRatio());
  // выводим значения газов в ppm
  Serial.print(" dat1: ");
  Serial.println(sens1.readMethane());
  Serial.print(" dat2: ");
  Serial.println(sens2.readMethane());
  Serial.print(" dat3: ");
  Serial.println(sens3.readMethane());
  delay(100);
  }
}
