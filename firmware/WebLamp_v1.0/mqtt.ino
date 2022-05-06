/*
  Протокол:
  GWL:0,ir  // пакет heartbeat, состояние ИК
  GWL:1,power,color,wink  // пакет данных
  GWL:2     // запрос настроек
*/

/*
  Функция шифрования с помощью XOR :-\
*/
template<class T> void xor_crypt(const char *key, T &data, int data_len)
{
  int key_len = strlen(key);
  for (int i = 0; i < data_len; i++)
    data[i] ^= key[ i % key_len ];
}

// опрашиваем mqtt
void mqttTick() {
  if (WiFi.status() != WL_CONNECTED) return;  // wifi не подключен
  if (!mqtt.connected()) connectMQTT();
  else {
    if (!startFlag) {
      startFlag = 1;
      char str[] = MQTT_HEADER "2";  // +2

      //если включено шифрование
      if (data.use_enc_phrase)
      {
        xor_crypt(data.enc_phrase, str, hLen + 1);
        DEBUGLN(str);
      }

      mqtt.publish(data.remote, str);
    }
  }
  mqtt.loop();
}

void connectMQTT() {
  // задаём случайный ID
  String id("WebLamp-");
  id += String(random(0xffffff), HEX);
  //DEBUGLN(id);
  // подписываемся на своё имя
  if (mqtt.connect(id.c_str())) mqtt.subscribe(data.local);
  delay(1000);
}

// тут нам прилетел пакет от удалённой лампы
void callback(char* topic, byte* payload, uint16_t len) {
  payload[len] = '\0';        // закрываем строку
  char* str = (char*)payload; // для удобства
  DEBUGLN(str);

  //если включено шифрование
  if (data.use_enc_phrase)
  {
    xor_crypt(data.enc_phrase, str, len);
    DEBUGLN(str);
  }
  
  // не наш пакет, выходим
  if (strncmp(str, MQTT_HEADER, hLen)) return;

  str += hLen;   // смещаемся для удобства чтения

  switch (getFromIndex(str, 0)) {
    case 0:   // heartbeat
      if (!IGNORE_PIR && getFromIndex(str, 1)) pirTmr.restart();
      break;

    case 1:   // управление
      data.power = getFromIndex(str, 1);
      data.color = getFromIndex(str, 2);
      if (getFromIndex(str, 3)) wink();
      break;

    case 2:   // запрос
      sendPacket();
      break;
  }

  onlineTmr.restart();  // перезапуск таймера онлайна
}

// отправляем пакет
void sendPacket() {
  // GWL:1,power,color,wink
  String s;
  s.reserve(10);
  s += MQTT_HEADER "1,";  // +1,
  s += data.power;
  s += ',';
  s += data.color;
  s += ',';
  s += winkFlag;
  winkFlag = 0;
  // отправляем

  //если включено шифрование
  if (data.use_enc_phrase)
  {
    xor_crypt(data.enc_phrase, s, s.length());
    DEBUGLN(s);
  }

  mqtt.publish(data.remote, s.c_str());
}

void heartbeat() {
  if (hbTmr.period()) {
    // GWL:0,pir
    char str[hLen + 4] = MQTT_HEADER "0,";  // +0,
    str[hLen + 2] = pirFlag + '0';
    pirFlag = 0;

    //если включено шифрование
    if (data.use_enc_phrase)
    {
      xor_crypt(data.enc_phrase, str, hLen + 3);
      DEBUGLN(str);
    }
    mqtt.publish(data.remote, str);
  }
}
