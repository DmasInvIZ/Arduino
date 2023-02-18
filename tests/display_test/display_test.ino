//#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);                                                       // инициализируем переменную lcd

unsigned long last_time;

bool flag = false;

void lcd_print(int column, int row, String text) {                                        // укороченая запись принта
  // формат записи lcd_print(номер_символа_в_строке, номер_строки, текст)
  
  lcd.setCursor(column, row);
  lcd.print(text);
  
}

void setup() {

  lcd.init();                                                                             // инициализируем дисплей
  lcd.backlight();                                                                        // включение подсветки

}

void loop() {

  lcd_print(4, 0, "Hello world!");
  lcd_print(3, 1, "Arduino rulez!");
  lcd.setCursor(0, 3);
  if (flag == false) {
    int counter = 0;
    while (counter != 20) {
      lcd.print("#");
      counter += 1;
      if (counter == 20) {flag = true;}
      delay(100);
    }
  }

  if (flag == true) {
    int counter = 0;
    lcd.setCursor(0, 3);
    while (counter != 20) {
      lcd.print(" ");
      counter += 1;
      if (counter == 20) {flag = false;}
      delay(100);
    }
  }
}
