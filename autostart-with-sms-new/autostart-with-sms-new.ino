/*
*2 конпка старт
*3 зажигание
*4 аксессуары
*5 иммобилайзер
*8 датчик ручника
*/

// #include <SoftwareSerial.h>


#define pin_ACC 4               //первое положение ключа
#define pin_ENGINE 3            //зажигание
#define pin_START 2             //кнопка старт 
#define pin_HANDBREAK 6         //ручной тормоз
#define engine_status_pin 7     //пин считывает статус двигателя ВКЛ/ВЫКЛ

bool engine_status;             //статус двигателя ВКЛ/ВЫКЛ




void setup() {
  Serial.begin(9600);
  Serial.println("Started");
  pinMode(pin_ACC, OUTPUT);
  pinMode(pin_ENGINE, OUTPUT);
  pinMode(pin_START, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

}
