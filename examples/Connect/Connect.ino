#include <MIIO.h>
MIIO miio(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  miio.begin("model", "mcu_version", "ble_pid");
}

void loop() {
  miio.loop();
}
