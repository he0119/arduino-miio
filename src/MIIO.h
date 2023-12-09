#ifndef _MIIO_H_
#define _MIIO_H_

#include "Arduino.h"

class MIIO
{
public:
  MIIO(Stream& serial);

  void begin(const char* model, const char* blePid, const char* mcuVersion);

  void begin(String model, String blePid, String mcuVersion);

  void loop();
private:
  Stream* _serial;
};

#endif
