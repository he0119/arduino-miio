#include "MIIO.h"
#include "miio/miio_define.h"

MIIO::MIIO(Stream& serial)
{
  _serial = &serial;
}

void MIIO::begin(const char* model, const char* blePid, const char* mcuVersion)
{
  _serial->setTimeout(_serialTimeoutMs);

  {
    int result = 0;
    do {
      result = MIIO_OK;
      // TODO: 实现基本设置逻辑
      // https://iot.mi.com/v2/new/doc/resources-and-services/personal-developer/embedded-dev#MCU%20程序开发

      delay(_pollIntervalMs);
    } while (result != MIIO_OK);
  }
}

void MIIO::begin(String model, String blePid, String mcuVersion)
{
  begin(model.c_str(), blePid.c_str(), mcuVersion.c_str());
}

void MIIO::loop()
{
  delay(_pollIntervalMs);
  while (true)
  {
    // TODO: 实现命令处理逻辑
  }

}

void MIIO::setSerialTimeout(unsigned long timeout)
{
  _serialTimeoutMs = timeout;
  _serial->setTimeout(_serialTimeoutMs);
}

void MIIO::setPollInterval(unsigned long interval)
{
  _pollIntervalMs = interval;
}
