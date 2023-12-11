#include <MIIO.h>
MIIO miio(Serial);

void setup() {
  Serial.begin(115200);

  miio.setPollInterval(5000);
  miio.setSerialTimeout(2000);
  miio.setReceiveRetry(3);

  miio.begin("perdev.switch.004", "18031", "0001");
  miio.onCommand(GET_PRO_STRING, getPropertiesCallback);
}

int getPropertiesCallback(char *cmd, size_t length) {
  DEBUG_MIIO("[USER]get properties: %s\n", cmd);

  auto *properties = property_value_new_boolean(true);

  miio.sendPropertyChanged(1, 1, properties);

  delete properties;

  return 0;
}

void loop() {
  miio.loop();
}
