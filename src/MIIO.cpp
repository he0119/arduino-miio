#include "MIIO.h"

MIIO::MIIO(Stream& serial)
{
  _serial = &serial;
}

void MIIO::begin(const char* model, const char* mcu_version, const char* ble_pid)
{
}

void MIIO::begin(String model, String mcu_version, String ble_pid)
{
  begin(model.c_str(), mcu_version.c_str(), ble_pid.c_str());
}

void MIIO::loop()
{
}

