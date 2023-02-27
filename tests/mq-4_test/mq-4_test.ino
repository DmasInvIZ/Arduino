// библиотека для работы с датчиками MQ (Troyka-модуль)
#include <TroykaMQ.h>
#include <LiquidCrystal_I2C.h>
 
// имя для пина, к которому подключен датчик
#define PIN_MQ4         A2
// имя для пина, к которому подключен нагреватель датчика
#define PIN_MQ4_HEATER  13

LiquidCrystal_I2C lcd(0x27, 20, 4);
 
// создаём объект для работы с датчиком
// и передаём ему номер пина выходного сигнала и нагревателя
MQ4 mq4(PIN_MQ4, PIN_MQ4_HEATER);
 
void setup()
{
  // открываем последовательный порт
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  // включаем нагреватель
  mq4.heaterPwrHigh();
  Serial.println("Heated sensor");
}
 
void loop()
{
  // если прошёл интервал нагрева датчика
  // и калибровка не была совершена
  if (!mq4.isCalibrated() && mq4.heatingCompleted()) {
    // выполняем калибровку датчика на чистом воздухе
    mq4.calibrate();
    // выводим сопротивление датчика в чистом воздухе (Ro) в serial-порт
    Serial.print("Ro = ");
    Serial.println(mq4.getRo());
    lcd.setCursor(3, 0);
    lcd.print(mq4.getRo());
    
  }
  // если прошёл интервал нагрева датчика
  // и калибровка была совершена
  if (mq4.isCalibrated() && mq4.heatingCompleted()) {
    // выводим отношения текущего сопротивление датчика
    // к сопротивлению датчика в чистом воздухе (Rs/Ro)
    Serial.print("Ratio: ");
    Serial.print(mq4.readRatio());
    lcd.setCursor(3, 1);
    lcd.print(mq4.readRatio());
    // выводим значения газов в ppm
  // выводим значения газов в ppm
  Serial.print(" Methane: ");
  Serial.print(mq4.readMethane());
  Serial.println(" ppm ");
  lcd.setCursor(3, 2);
  lcd.print(mq4.readMethane());
  delay(100);
  }
}
