#ifndef _MIIO_H_
#define _MIIO_H_

#include "Arduino.h"

class MIIO
{
private:
  Stream* _serial;
public:
  MIIO(Stream& serial);

  void begin(const char* model, const char* ble_pid, const char* mcu_version);

  void begin(String model, String ble_pid, String mcu_version);

  void loop();
};

#endif
