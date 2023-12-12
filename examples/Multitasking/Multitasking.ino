#include <SerialMIIO.h>
SerialMIIO miio(Serial);

TaskHandle_t Task1;
TaskHandle_t Task2;
unsigned long last = 0;

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 1);
  delay(10);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
  delay(10);
}

void Task1code(void *parameter) {
  miio.setPollInterval(5000);
  miio.setSerialTimeout(200);
  miio.setReceiveRetry(1);

  miio.begin("perdev.switch.004", "18031", "0001");
  for (;;) {
    miio.loop();
  }
}

void Task2code(void *parameter) {
  for (;;) {
    if (millis() - last > 1000) {
      Serial.println("Hello World!");
      last = millis();
    }
    vTaskDelay(1);
  }
}

void loop() {
}
