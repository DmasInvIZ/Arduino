#include <GyverButton.h>

#define BTN_PIN D7      // кнопка
#define PWM_PIN D6      // MOSFET (LED лента)

GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);

bool ledState = false;
uint8_t brightness = 0;
const uint8_t MAX_BRIGHT = 1023;
const uint8_t FADE_STEP = 3;     // скорость плавности
const uint16_t FADE_DELAY = 50;  // мс между шагами

void setup() {
  Serial.begin(115200);

  pinMode(PWM_PIN, OUTPUT);
  analogWriteRange(1023);
  analogWriteFreq(1000);   // без писка

  btn.setDebounce(50);
  btn.setTimeout(500);

  analogWrite(PWM_PIN, 0); // старт: выкл

  Serial.println("Тест ленты: кнопка D7, MOSFET D6");
}

void loop() {
  btn.tick();

  if (btn.isClick()) {
    ledState = !ledState;

    if (ledState) {
      fadeIn();
    } else {
      fadeOut();
    }
  }
}

void fadeIn() {
  Serial.println("Плавное ВКЛ");
  for (int i = 0; i <= MAX_BRIGHT; i += FADE_STEP) {
    brightness = i;
    analogWrite(PWM_PIN, brightness);
    delay(FADE_DELAY);
  }
}

void fadeOut() {
  Serial.println("Плавное ВЫКЛ");
  for (int i = brightness; i >= 0; i -= FADE_STEP) {
    analogWrite(PWM_PIN, i);
    delay(FADE_DELAY);
  }
  brightness = 0;
}
