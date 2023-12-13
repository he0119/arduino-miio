#include <SerialMIIO.h>
SerialMIIO miio(Serial);

TaskHandle_t MIIOTask;

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(MIIOLoop, "miio", 10000, NULL, 1, &MIIOTask, 1);
  delay(10);
}

void MIIOLoop(void *parameter) {
  miio.begin("perdev.switch.004", "18031", "0001");
  for (;;) {
    miio.loop();
  }
}

unsigned long last = 0;

void loop() {
  if (millis() - last > 1000) {
    Serial.println("Hello World!");
    last = millis();
  }
  vTaskDelay(1);
}
