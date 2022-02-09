/*
  Скетч к проекту "WebLamp"
  - Страница проекта (схемы, описания): https://alexgyver.ru/weblamp/
  - Исходники на GitHub: https://github.com/AlexGyver/WebLamp
  Проблемы с загрузкой? Читай гайд для новичков: https://alexgyver.ru/arduino-first/
  AlexGyver, AlexGyver Technologies, 2022

  1.0 - релиз
*/

#define LED_PIN D1    // пин ленты
#define BTN_PIN D2    // пин кнопки
#define PIR_PIN D5    // пин PIR (ИК датчика)
#define LED_AMOUNT 30 // кол-вл светодиодов
#define BTN_LEVEL 1   // 1 - кнопка подключает VCC, 0 - подключает GND
#define USE_PIR 1     // 1 - использовать PIR (ИК датчик) на этой лампе
#define IGNORE_PIR 0  // 1 - игнорировать сигнал PIR (ИК датчика) с удалённой лампы

/*
  Запуск:
  Клик или >15 секунд при анимации подклчения: запустить точку доступа
  Кнопка сохранить или клик: перезагрузить систему

  Анимация:
  - Мигает зелёным: подключение к роутеру
  - Мигает синим: запущена точка доступа WLamp <IP>

  Работа:
  1 клик: вкл/выкл
  2 клика: сменить цвет
  3 клика: подмигнуть
  Удержание: сменить яркость
*/

// ============= ВСЯКОЕ =============
#define MQTT_HEADER "GWL:"  // заголовок пакета данных

// ============= БИБЛЫ =============
#include <GyverPortal.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEManager.h>
#include <FastLED.h>
#define EB_STEP 100   // период step шага кнопки
#include <EncButton.h>
#include "Timer.h"

// ============= ДАННЫЕ =============
#if 1
#define DEBUG(x) Serial.print(x)
#define DEBUGLN(x) Serial.println(x)
#else
#define DEBUG(x)
#define DEBUGLN(x)
#endif

struct LampData {
  char ssid[32] = "";
  char pass[32] = "";
  char local[20] = "AG_lamp_1";
  char remote[20] = "AG_lamp_2";
  char host[32] = "broker.mqttdashboard.com";
  uint16_t port = 1883;
  uint8_t ip[4] = {0, 0, 0, 0};

  bool power = 1;
  uint8_t bright = 50;
  uint8_t color = 0;
};

LampData data;
EncButton<EB_TICK, BTN_PIN> btn;
CRGB leds[LED_AMOUNT];
WiFiClient espClient;
PubSubClient mqtt(espClient);
GyverPortal portal;
EEManager memory(data);
bool pirFlag = 0;
bool winkFlag = 0;
bool startFlag = 0;
const uint8_t hLen = strlen(MQTT_HEADER);

Timer onlineTmr(18000, false);  // 18 секунд таймаут онлайна
Timer pirTmr(60000, false);     // 1 минута таймаут пира
Timer hbTmr(8000);              // 8 секунд период отправки пакета

void setup() {
  startup();      // запускаем всё
}

void loop() {
  if (USE_PIR && digitalRead(PIR_PIN)) pirFlag = 1;  // опрос ИК датчика
  
  heartbeat();    // отправляем пакет что мы онлайн
  memory.tick();  // проверяем обновление настроек
  animation();    // эффект ленты
  buttonTick();   // действия кнопки
  mqttTick();     // проверяем входящие
  portal.tick();  // пинаем портал
  checkPortal();  // проверяем действия
}
