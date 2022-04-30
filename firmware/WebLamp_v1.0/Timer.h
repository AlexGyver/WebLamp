// простенький таймер на миллис

#pragma once
struct Timer {
  // указать период, опционально статус (запущен/не запущен)
  Timer (uint32_t nprd, bool nstate = true) {
    state = nstate;
    setPeriod(nprd);
    tmr = millis();
  }

  // сменить период
  void setPeriod(uint32_t nprd) {
    prd = nprd;
  }

  // перезапустить
  void restart() {
    tmr = millis();
    state = true;
  }

  // время с рестарта вышло
  bool elapsed() {
    return (!state || check());
  }

  // периодический таймер
  bool period() {
    if (state && check()) {
      restart();
      return 1;
    } return 0;
  }

  bool check() {
    return millis() - tmr >= prd;
  }

  uint32_t tmr, prd;
  bool state = 1;
};
