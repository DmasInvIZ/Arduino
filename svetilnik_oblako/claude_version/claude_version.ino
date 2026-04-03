#include <GyverButton.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

// ===== Пины =====
#define BTN_PIN D7
#define PWM_PIN D6

GButton btn(BTN_PIN, LOW_PULL, NORM_OPEN);
ESP8266WebServer server(80);
bool serverRunning = false;
uint32_t serverCheckTimer = 0;


// ===== WiFi =====
const char* ssid     = "AwesomePeople";
const char* password = "375295689321";

// ===== EEPROM =====
#define EEPROM_SIZE 8  // onH, onM, offH, offM

// ===== Расписание =====
int onHour  = 0;
int onMin   = 0;
int offHour = 0;
int offMin  = 0;

// ===== Яркость =====
bool ledState = false;
bool dimUp = false;

uint16_t brightness = 0;
uint16_t targetBrightness = 800;
const uint16_t MAX_BRIGHT = 1023;
const uint16_t MIN_BRIGHT = 50;

const uint16_t STEP = 5;       // шаг изменения
const uint16_t INTERVAL = 15;  // интервал плавного изменения (мс)

// Таймеры
uint32_t fadeTimer = 0;
uint32_t scheduleTimer = 0;

// ---------------- WIFI ----------------
void connectWiFi() {
  Serial.print("Подключение к WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi подключен, IP: " + WiFi.localIP().toString());
}

// ---------------- NTP ----------------
void setupTime() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Синхронизация времени");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Время синхронизировано");
}

// ---------------- EEPROM ----------------
void loadSchedule() {
  onHour  = EEPROM.read(0);
  onMin   = EEPROM.read(1);
  offHour = EEPROM.read(2);
  offMin  = EEPROM.read(3);

  if (onHour > 23) onHour = 0;
  if (onMin > 59) onMin = 0;
  if (offHour > 23) offHour = 0;
  if (offMin > 59) offMin = 0;

  Serial.printf("Расписание: ВКЛ %02d:%02d / ВЫКЛ %02d:%02d\n", onHour, onMin, offHour, offMin);
}

void saveSchedule() {
  bool changed = false;
  if (EEPROM.read(0) != onHour)  { EEPROM.write(0, onHour); changed = true; }
  if (EEPROM.read(1) != onMin)   { EEPROM.write(1, onMin); changed = true; }
  if (EEPROM.read(2) != offHour) { EEPROM.write(2, offHour); changed = true; }
  if (EEPROM.read(3) != offMin)  { EEPROM.write(3, offMin); changed = true; }
  if (changed) EEPROM.commit();

  Serial.println("Расписание сохранено (если изменилось)");
}

// ---------------- ПЛАВНАЯ ЯРКОСТЬ ----------------
void fadeTo(uint16_t target) {
  targetBrightness = constrain(target, 0, MAX_BRIGHT);
}

void updateBrightness() {
  if (millis() - fadeTimer < INTERVAL) return;
  fadeTimer = millis();

  if (brightness == targetBrightness) return;

  if (brightness < targetBrightness) {
    brightness += STEP;
    if (brightness > targetBrightness) brightness = targetBrightness;
  } else {
    brightness -= STEP;
    if (brightness < targetBrightness) brightness = targetBrightness;
  }

  analogWrite(PWM_PIN, brightness);
}

// ---------------- РЕГУЛИРОВКА ПО КНОПКЕ ----------------
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

// ---------------- ВЕБ-СТРАНИЦА ----------------
String getPage() {
  String page = "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  page += "<style>body{font-family:Arial;text-align:center;}button,input{font-size:16px;margin:5px;}</style></head><body>";
  page += "<h1>Светильник</h1>";
  page += "<a href='/on'><button>ВКЛ</button></a>";
  page += "<a href='/off'><button>ВЫКЛ</button></a><br>";

  page += "<form action='/setBrightness'>";
  page += "Яркость: <input type='range' name='val' min='50' max='1023' value='" + String(brightness) + "'>";
  page += "<input type='submit' value='OK'></form><br>";

  page += "<form action='/setTime'>";
  page += "Вкл: <input name='onH' value='" + String(onHour) + "' size=2>:";
  page += "<input name='onM' value='" + String(onMin) + "' size=2><br>";
  page += "Выкл: <input name='offH' value='" + String(offHour) + "' size=2>:";
  page += "<input name='offM' value='" + String(offMin) + "' size=2><br>";
  page += "<input type='submit' value='Сохранить'></form>";
  page += "</body></html>";
  return page;
}

// ---------------- ОБРАБОТЧИКИ ----------------
void handleRoot() { server.send(200, "text/html", getPage()); }

void handleOn() {
  ledState = true;
  fadeTo(targetBrightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  ledState = false;
  fadeTo(0);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBrightness() {
  if (server.hasArg("val"))
    targetBrightness = constrain(server.arg("val").toInt(), MIN_BRIGHT, MAX_BRIGHT);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetTime() {
  if (server.hasArg("onH")) onHour  = constrain(server.arg("onH").toInt(), 0, 23);
  if (server.hasArg("onM")) onMin   = constrain(server.arg("onM").toInt(), 0, 59);
  if (server.hasArg("offH")) offHour = constrain(server.arg("offH").toInt(), 0, 23);
  if (server.hasArg("offM")) offMin  = constrain(server.arg("offM").toInt(), 0, 59);
  saveSchedule();
  server.sendHeader("Location", "/");
  server.send(303);
}

//контроль работы веб-сервера
void checkServerStatus () {
  if (millis() - serverCheckTimer >= 5000) {  // каждые 5 секунд
    serverCheckTimer = millis();
    // Проверка WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️  Потеря WiFi, пытаюсь переподключиться...");
      connectWiFi();
    }
    // Проверка нашего флага работы сервера
    if (!serverRunning) {
      Serial.println("⚠️  Сервер не активен. Перезапуск...");
      server.stop();       // на всякий случай
      delay(50);
      server.begin();
      serverRunning = true;
      Serial.println("✅ Сервер снова запущен");
     }
   }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  pinMode(PWM_PIN, OUTPUT);
  analogWriteFreq(1000);
  analogWrite(PWM_PIN, 0);

  btn.setDebounce(50);
  btn.setTimeout(500);

  EEPROM.begin(EEPROM_SIZE);
  loadSchedule();

  connectWiFi();
  setupTime();

  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/setBrightness", handleBrightness);
  server.on("/setTime", handleSetTime);
  server.begin();
  serverRunning = true;
  Serial.println("Веб-сервер запущен");
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  btn.tick();

  if (btn.isClick()) {
    ledState = !ledState;
    fadeTo(ledState ? targetBrightness : 0);
  }

  if (btn.isHolded()) dimUp = !dimUp;
  if (btn.isHold() && ledState) changeBrightness();

  updateBrightness();

  if (millis() - scheduleTimer >= 1000) {
    scheduleTimer = millis();
    checkSchedule();
  }

  checkServerStatus();
  ArduinoOTA.handle();
}
