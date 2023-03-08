/*
 * Тест дисплея ардуино, подключается по I2C
 * Ардуино A4 - дисплей SDA, ардуино A5 - дисплей SCL
 */

//#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// размер дисплея
#define collumns   16   // символов в строке
#define rows       2    // строк

LiquidCrystal_I2C lcd(0x27, collumns, rows);                                              // инициализируем переменную lcd

int count = 0;

void setup() {

  lcd.init();                                                                             // инициализируем дисплей
  lcd.backlight();                                                                        // включение подсветки
  lcd.setCursor(0, 0);
  lcd.print("It works!");
}

void loop() {
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print(count);
    count++;
    delay(1000);
  }
}
