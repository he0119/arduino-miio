#include <MIIO.h>
MIIO miio(Serial);

void setup() {
  Serial.begin(115200);

  miio.setPollInterval(5000);
  miio.setSerialTimeout(2000);
  miio.setReceiveRetry(3);

  miio.begin("perdev.switch.004", "18031", "0001");
  miio.onCommand(GET_PRO_STRING, get_properties_callback);
}

int get_properties_callback(char* cmd, size_t length) {
  DEBUG_MIIO("[USER]get properties: %s\n", cmd);
  miio.sendStr("result 1 1 0");
  return 0;
}

void loop() {
  miio.loop();
}
