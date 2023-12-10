#include <MIIO.h>
MIIO miio(Serial);

void setup() {
  Serial.begin(115200);

  miio.setPollInterval(5000);
  miio.setSerialTimeout(1000);

  miio.begin("perdev.switch.004", "18031", "0001");
}

void loop() {
  miio.loop();
}
