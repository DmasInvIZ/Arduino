#include "GyverButton.h"

#define BTN_PIN 14   // кнопка подключена сюда
#define light_btn 5
#define n_light_btn 4

GButton butt1(BTN_PIN);
//EncButton<EB_TICK, BTN_PIN> butt1;

bool light_is_on = false;
bool n_light_is_on = false;
int value;
int light_brightness = 255;

void setup() {
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  Serial.println("Started");
}

void loop() {
  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt1.isSingle() && light_is_on == false) {
    analogWrite(light_btn, light_brightness);
    light_is_on = true;
    Serial.println("light ON");
  } else if (butt1.isSingle() && light_is_on == true){
    analogWrite(light_btn, LOW);
    light_is_on = false;
    Serial.println("light OFF");
  }
  
    if (butt1.isDouble() && n_light_is_on == false) {
    digitalWrite(n_light_btn, HIGH);
    n_light_is_on = true;
    Serial.println("n light ON");
  } else if (butt1.isDouble() && n_light_is_on == true){
    digitalWrite(n_light_btn, LOW);
    n_light_is_on = false;
    Serial.println("n light OFF");
  }
  
  if (butt1.isHold() && butt1.getHoldClicks() == 0) {
    light_brightness = light_brightness - 10;
    Serial.println(light_brightness);
    analogWrite(light_btn, light_brightness);
    delay(200);
  }
  if (butt1.isHold() && butt1.getHoldClicks() == 1) {
    light_brightness = light_brightness + 10;
    Serial.println(light_brightness);
    analogWrite(light_btn, light_brightness);
    delay(200);
  }
  
  light_brightness = constrain(light_brightness, 30, 255);
}
