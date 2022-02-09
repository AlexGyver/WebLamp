// опрашиваем mqtt
void mqttTick() {
  if (WiFi.status() != WL_CONNECTED) return;  // wifi не подключен
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
}

void connectMQTT() {
  // задаём случайный ID
  String id("WebLamp-");
  id += String(random(0xffffff), HEX);
  DEBUGLN(id);
  // подписываемся на своё имя
  if (mqtt.connect(id.c_str())) mqtt.subscribe(data.local);
  delay(1000);
}

// тут нам прилетел пакет от удалённой лампы
void callback(char* topic, byte* payload, uint16_t len) {
  payload[len] = '\0';        // закрываем строку
  char* str = (char*)payload; // для удобства
  //DEBUGLN(str);
  // не наш пакет, выходим
  if (strncmp(str, MQTT_HEADER, strlen(MQTT_HEADER))) return;

  str += strlen(MQTT_HEADER);   // смещаемся для удобства чтения

  // читаем что у нас там
  data.power = getFromIndex(str, 0);
  data.color = getFromIndex(str, 1);
  if (getFromIndex(str, 2)) wink();
  if (!IGNORE_PIR && getFromIndex(str, 3)) pirTmr.restart();

  onlineTmr.restart();  // перезапуск таймера онлайна
}

// отправляем пакет
void sendPacket() {
  // GWL:power,color,wink,ir
  String s;
  s.reserve(15);
  s += "GWL:";
  s += data.power;
  s += ',';
  s += data.color;
  s += ',';
  s += winkFlag;
  s += ',';
  s += pirFlag;
  winkFlag = pirFlag = 0;   // сбрасываем флаги

  // отправляем
  mqtt.publish(data.remote, s.c_str());
  
  // принудительно сбрасываем таймер отправки, 
  // чтобы не отправлять слишком часто
  hbTmr.restart();
}
