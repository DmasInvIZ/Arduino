#include <GyverButton.h>

#define BTN_PIN D7
#define PWM_PIN D6

GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);

bool ledState = false;

uint16_t brightness = 0;        // текущая яркость
uint16_t targetBrightness = 800; // сохранённая яркость

bool dimUp = false; // направление регулировки

const uint16_t MAX_BRIGHT = 1023;
const uint16_t MIN_BRIGHT = 50;

const uint8_t STEP = 5;
const uint16_t INTERVAL = 15;

void setup() {
  Serial.begin(115200);

  pinMode(PWM_PIN, OUTPUT);
  analogWriteRange(1023);
  analogWriteFreq(1000);

  btn.setDebounce(50);
  btn.setTimeout(500);

  analogWrite(PWM_PIN, 0);
}

// ---------------- ОСНОВНОЙ ЦИКЛ ----------------
void loop() {
  btn.tick();

  // 🔘 КЛИК — включить / выключить
  if (btn.isClick()) {
    ledState = !ledState;

    if (ledState) {
      fadeTo(targetBrightness); // включаемся до сохранённого уровня
    } else {
      fadeTo(0); // ВАЖНО: уходим в полный ноль
    }
  }

  // ✊ начало удержания — меняем направление
  if (btn.isHolded()) {
    dimUp = !dimUp;
  }

  // ✊ удержание — регулируем яркость
  if (btn.isHold() && ledState) {
    changeBrightness();
  }
}

// ---------------- ПЛАВНОЕ ИЗМЕНЕНИЕ ----------------
void fadeTo(uint16_t target) {

  // вверх
  if (brightness < target) {
    for (uint16_t i = brightness; i < target; i += STEP) {
      analogWrite(PWM_PIN, i);
      delay(INTERVAL);
    }
  }
  // вниз
  else {
    for (int i = brightness; i > target; i -= STEP) {
      analogWrite(PWM_PIN, i);
      delay(INTERVAL);
    }
  }

  // 🔥 гарантированно устанавливаем конечное значение
  analogWrite(PWM_PIN, target);
  brightness = target;
}

// ---------------- РЕГУЛИРОВКА ----------------
void changeBrightness() {
  static uint32_t timer;

  if (millis() - timer >= INTERVAL) {
    timer = millis();

    if (dimUp) {
      brightness += STEP;
      if (brightness > MAX_BRIGHT) brightness = MAX_BRIGHT;
    } else {
      brightness -= STEP;

      // ❗ ограничение только для режима регулировки
      if (brightness < MIN_BRIGHT) brightness = MIN_BRIGHT;
    }

    analogWrite(PWM_PIN, brightness);

    // 💾 запоминаем уровень
    targetBrightness = brightness;
  }
}
