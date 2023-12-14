#include <SerialMIIO.h>
SerialMIIO miio(Serial);

void setup() {
  Serial.begin(115200);

  miio.begin("perdev.switch.004", "18031", "0001");
}

unsigned long last = 0;

void loop() {
  // miio.loop();

  if (millis() - last > 1000) {
    Serial.println("Hello World!");
    last = millis();
  }
}
