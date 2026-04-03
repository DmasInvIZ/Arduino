#include <GyverButton.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h> // для сохранения расписания
#include <ArduinoOTA.h>

#define BTN_PIN D7
#define PWM_PIN D6

GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);
ESP8266WebServer server(80);

// ===== WiFi =====
const char* ssid = "AwesomePeople";
const char* password = "375295689321";

// ======= НАСТРОЙКА РАСПИСАНИЯ =======
int onHour = 00;
int onMin  = 00;
int offHour = 00;
int offMin  = 00;

// ===== EEPROM =====
#define EEPROM_SIZE 8 // хватит для 4 байт: onH, onM, offH, offM

// ====================================

bool ledState = false;

uint16_t brightness = 0;
uint16_t targetBrightness = 800;

bool dimUp = false;

const uint16_t MAX_BRIGHT = 1023;
const uint16_t MIN_BRIGHT = 50;

const uint8_t STEP = 5;
const uint16_t INTERVAL = 15;

uint32_t scheduleTimer = 0;

// ---------------- WIFI ----------------
void connectWiFi() {
  Serial.print("Подключение к WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi подключен");
  Serial.println("IP адрес: " + WiFi.localIP().toString());
}

// ---------------- NTP ----------------
void setupTime() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Синхронизация времени");
  while (time(nullptr) < 100000) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nВремя синхронизировано");
}

// ---------------- EEPROM ----------------
void loadSchedule() {
  onHour  = EEPROM.read(0);
  onMin   = EEPROM.read(1);
  offHour = EEPROM.read(2);
  offMin  = EEPROM.read(3);

  // проверяем корректность
  if (onHour > 23) onHour = 19;
  if (onMin  > 59) onMin  = 30;
  if (offHour > 23) offHour = 20;
  if (offMin  > 59) offMin  = 34;

  Serial.println("Расписание загружено из EEPROM");
}

void saveSchedule() {
  EEPROM.write(0, onHour);
  EEPROM.write(1, onMin);
  EEPROM.write(2, offHour);
  EEPROM.write(3, offMin);
  EEPROM.commit();
  Serial.println("Расписание сохранено в EEPROM");
}

// ---------------- РАСПИСАНИЕ ----------------
void checkSchedule() {
  static int lastMinute = -1;
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);

  int h = t->tm_hour;
  int m = t->tm_min;

  if (m == lastMinute) return;
  lastMinute = m;

  if (h == onHour && m == onMin && !ledState) {
    ledState = true;
    fadeTo(targetBrightness);
    Serial.println("Авто ВКЛ");
  }
  if (h == offHour && m == offMin && ledState) {
    ledState = false;
    fadeTo(0);
    Serial.println("Авто ВЫКЛ");
  }
}

// ---------------- ПЛАВНОСТЬ ----------------
void fadeTo(uint16_t target) {
  if (brightness < target) {
    for (uint16_t i = brightness; i < target; i += STEP) {
      analogWrite(PWM_PIN, i);
      delay(INTERVAL);
    }
  } else {
    for (int i = brightness; i > target; i -= STEP) {
      analogWrite(PWM_PIN, i);
      delay(INTERVAL);
    }
  }
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
      if (brightness < MIN_BRIGHT) brightness = MIN_BRIGHT;
    }
    analogWrite(PWM_PIN, brightness);
    targetBrightness = brightness;
  }
}

// ---------------- ВЕБ-ИНТЕРФЕЙС ----------------
String getPage() {
  String page = "<html><head><meta charset='utf-8'>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<style>";
  page += "body { display: flex; justify-content: center; align-items: center; height: 100vh; ";
  page += "font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; }";
  page += ".container { background: #fff; padding: 20px 30px; border-radius: 12px; ";
  page += "box-shadow: 0 4px 10px rgba(0,0,0,0.2); width: 90%; max-width: 400px; text-align: center; }";
  page += "h2, h3 { margin: 15px 0; }";
  page += "button, input[type='submit'] { padding: 10px 20px; margin: 5px; font-size: 16px; border-radius: 8px; border: none; background-color: #4CAF50; color: white; }";
  page += "input[type='range'] { width: 100%; margin: 10px 0; }";
  page += "input[type='text'], input[type='number'] { width: 50px; font-size: 16px; text-align: center; margin: 2px; }";
  page += "</style>";
  page += "</head><body>";

  page += "<div class='container'>";
  page += "<h2>Светильник</h2>";

  // кнопки
  page += "<a href='/on'><button>ВКЛ</button></a>";
  page += "<a href='/off'><button>ВЫКЛ</button></a>";

  // яркость
  page += "<h2>Яркость</h2>";
  page += "<form action='/setBrightness'>";
  page += "<input type='range' name='val' min='50' max='1023' value='" + String(brightness) + "'>";
  page += "<input type='submit' value='OK'>";
  page += "</form>";

  // расписание
  page += "<h3>Расписание</h3>";
  page += "<form action='/setTime'>";
  page += "Включение: <input name='onH' value='" + String(onHour) + "' size=2>:" +
          "<input name='onM' value='" + String(onMin) + "' size=2><br>";
  page += "Выключение: <input name='offH' value='" + String(offHour) + "' size=2>:" +
          "<input name='offM' value='" + String(offMin) + "' size=2><br>";
  page += "<input type='submit' value='Сохранить'>";
  page += "</form>";

  page += "</div>"; // конец container
  page += "</body></html>";
  return page;
}

// ---------------- ОБРАБОТЧИКИ ----------------
void handleSetTime() {
  if (server.hasArg("onH")) onHour = constrain(server.arg("onH").toInt(), 0, 23);
  if (server.hasArg("onM")) onMin  = constrain(server.arg("onM").toInt(), 0, 59);
  if (server.hasArg("offH")) offHour = constrain(server.arg("offH").toInt(), 0, 23);
  if (server.hasArg("offM")) offMin  = constrain(server.arg("offM").toInt(), 0, 59);

  saveSchedule(); // <-- сохраняем расписание в EEPROM

  server.sendHeader("Location", "/");
  server.send(303);
}

// остальное как было
void handleRoot() { server.send(200, "text/html", getPage()); }
void handleOn() { ledState = true; fadeTo(targetBrightness); server.sendHeader("Location", "/"); server.send(303); }
void handleOff() { ledState = false; fadeTo(0); server.sendHeader("Location", "/"); server.send(303); }
void handleBrightness() {
  if (server.hasArg("val")) {
    brightness = server.arg("val").toInt();
    targetBrightness = brightness;
    analogWrite(PWM_PIN, brightness);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  ArduinoOTA.begin();

  pinMode(PWM_PIN, OUTPUT);
  analogWriteRange(1023);
  analogWriteFreq(1000);

  btn.setDebounce(50);
  btn.setTimeout(500);

  analogWrite(PWM_PIN, 0);

  EEPROM.begin(EEPROM_SIZE);
  loadSchedule(); // <-- загружаем расписание из EEPROM

  connectWiFi();
  setupTime();
  
  Serial.println("=========Расписание==========");
  Serial.printf(
  "Включение в  %02d:%02d\nВыключение в %02d:%02d\n",
  onHour, onMin,
  offHour, offMin
  );
  Serial.println("=============================");

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/setBrightness", handleBrightness);
  server.on("/setTime", handleSetTime);

  server.begin();
  Serial.println("Веб сервер запущен");

  //после старта моргнет
  analogWrite(PWM_PIN, 300);
  delay(200);
  analogWrite(PWM_PIN, LOW);
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  btn.tick();

  if (btn.isClick()) {
    ledState = !ledState;
    if (ledState) fadeTo(targetBrightness);
    else fadeTo(0);
  }

  if (btn.isHolded()) dimUp = !dimUp;
  if (btn.isHold() && ledState) changeBrightness();

  if (millis() - scheduleTimer >= 1000) {
    scheduleTimer = millis();
    checkSchedule();
  }
}
