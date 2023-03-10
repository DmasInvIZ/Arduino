#include <SoftwareSerial.h>
#include <Sim800l.h>
#include <LiquidCrystal_I2C.h>

Sim800l Sim800l;
SoftwareSerial SIM800(8, 9);        // 8 - RX Arduino (TX SIM800L), 9 - TX Arduino (RX SIM800L)
LiquidCrystal_I2C lcd(0x27, 16, 2);  

unsigned long last_time;
String txt;
String cmd;
int integer = 112;

void setup() {
  Serial.begin(9600);               // Скорость обмена данными с компьютером
  Serial.println("Start!");
  SIM800.begin(9600);               // Скорость обмена данными с модемом
  SIM800.println("AT");
  Sim800l.begin();
  Serial.println("OK");
  lcd.init();
  lcd.backlight();
}

void loop() {
  Serial.println("test");
  if (millis() - last_time > 5000) {
    cmd = Sim800l.sendATCommand("AT+CBC", true);
    txt = cmd.substring(cmd.indexOf(",") + 1, cmd.length());
    int bat = txt.toInt();
    Serial.println("send at");
    Serial.print(bat);
    Serial.println("%");
    lcd.setCursor(13, 1);
    lcd.print(bat);
    lcd.print("%");
    delay(10000);
    last_time = millis();
  }
}
