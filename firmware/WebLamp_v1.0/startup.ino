void startup() {
  delay(1000);
  Serial.begin(9600);         // запускаем сериал для отладки
  portal.attachBuild(build);  // подключаем интерфейс

  EEPROM.begin(sizeof(data) + 1); // +1 на ключ
  memory.begin(0, 'a');           // запускаем менеджер памяти

  // я хз, хранить IPAddress в памяти приводит к exception
  // так что вытаскиваем в IPAddress
  IPAddress ip = IPAddress(data.ip[0], data.ip[1], data.ip[2], data.ip[3]);

  // запускаем ленту
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_AMOUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  FastLED.show();

  // настраиваем уровень кнопки
  btn.setButtonLevel(BTN_LEVEL);

  // таймер на 2 секунды перед подключением,
  // чтобы юзер успел кликнуть если надо
  Timer tmr(2000);
  while (!tmr.period()) {
    loadAnimation(CRGB::Green);    // анимация подключения
    btn.tick();
    if (btn.click()) localPortal(ip); // клик - запускаем портал
    // дальше код не пойдёт, уйдем в перезагрузку
  }

  // юзер не кликнул, пытаемся подключиться к точке
  Serial.println("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(data.ssid, data.pass);
  
  tmr.setPeriod(15000);
  tmr.restart();

  while (WiFi.status() != WL_CONNECTED) {
    loadAnimation(CRGB::Green);      // анимация подключения
    btn.tick();
    // если клик по кнопке или вышел таймаут
    if (btn.click() || tmr.period()) {
      WiFi.disconnect();  // отключаемся
      localPortal(ip);    // открываем портал
      // дальше код не пойдёт, уйдем в перезагрузку
    }
  }
  FastLED.clear();
  FastLED.show();

  Serial.print(F("Connected! IP: "));
  Serial.println(WiFi.localIP());

  // переписываем удачный IP себе в память
  if (ip != WiFi.localIP()) {
    ip = WiFi.localIP();
    for (int i = 0; i < 4; i++) data.ip[i] = ip[i];
    memory.update();
  }

  // стартуем вебсокет
  mqtt.setServer(data.host, data.port);
  mqtt.setCallback(callback);
  randomSeed(micros());

  // стартуем портал
  portal.start();

  FastLED.setBrightness(data.bright);
}
