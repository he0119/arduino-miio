#include "MIIO.h"

MIIO::MIIO(Stream& serial)
{
  _serial = &serial;
}

void MIIO::begin(const char* model, const char* blePid, const char* mcuVersion)
{
}

void MIIO::begin(String model, String blePid, String mcuVersion)
{
  begin(model.c_str(), blePid.c_str(), mcuVersion.c_str());
}

void MIIO::loop()
{
}

