void build() {
  String s;
  BUILD_BEGIN(s);

  add.THEME(GP_DARK);
  add.AJAX_UPDATE("ledL,ledR,ledP,sw,br,col", 2000);

  add.LABEL("STATUS");
  add.BLOCK_BEGIN();
  add.LABEL("Local:");
  add.LED_GREEN("ledL", mqtt.connected());
  add.LABEL("Remote:");
  add.LED_GREEN("ledR", !onlineTmr.elapsed());
  add.BREAK();
  add.LABEL("Remote PIR:");
  add.LED_RED("ledP", (!pirTmr.elapsed() && !onlineTmr.elapsed()));
  add.BLOCK_END();

  add.LABEL("SETTINGS");
  add.BLOCK_BEGIN();
  add.LABEL("Power:");
  add.SWITCH("sw", data.power);
  add.BREAK();
  add.SLIDER("br", "Bright:", data.bright, 0, 255);
  add.SLIDER("col", "Color:", data.color, 0, 255);
  add.BLOCK_END();

  add.FORM_BEGIN("/save");

  add.LABEL("WIFI");
  add.BLOCK_BEGIN();
  add.TEXT("ssid", "SSID", data.ssid);
  add.BREAK();
  add.PASS("pass", "Password", data.pass);
  add.BLOCK_END();

  add.LABEL("MQTT");
  add.BLOCK_BEGIN();
  add.TEXT("local", "Local Name", data.local);
  add.BREAK();
  add.TEXT("remote", "Remote Name", data.remote);
  add.BREAK();
  add.TEXT("host", "Host", data.host);
  add.BREAK();
  add.NUMBER("port", "Port", data.port);
  add.BLOCK_END();
  add.SUBMIT("Save");

  add.FORM_END();

  BUILD_END();
}
