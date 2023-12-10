#ifndef _MIIO_H_
#define _MIIO_H_

#include "Arduino.h"
#include "miio/miio_define.h"

#ifndef NODEBUG_MIIO
#ifdef DEBUG_ESP_PORT
#define DEBUG_MIIO(...)               \
    {                                       \
        DEBUG_ESP_PORT.printf(__VA_ARGS__); \
        DEBUG_ESP_PORT.flush();             \
    }
#endif
#endif

#ifndef DEBUG_MIIO
#define DEBUG_MIIO(...)
#ifndef NODEBUG_MIIO
#define NODEBUG_MIIO
#endif
#endif

class MIIO
{
public:
  MIIO(Stream& serial);

  void begin(const char* model, const char* blePid, const char* mcuVersion);

  void begin(String model, String blePid, String mcuVersion);

  void loop();

  /* 串口超时

  单位 ms
  */
  void setSerialTimeout(unsigned long timeout);

  /* 轮询间隔

  时间范围要求 100~200ms，推荐的循环周期为 200ms
  https://iot.mi.com/v2/new/doc/embedded-dev/module-dev/function-dev/mcu-dev#命令规范
  */
  void setPollInterval(unsigned long interval);

  void setReceiveRetry(unsigned int retry);

  size_t sendStr(const char* str);

  size_t sendStr(String str);

  size_t sendStrWaitAck(const char* str);

  size_t sendStrWaitAck(String str);

  size_t recvStr(char* buffer, size_t length);

private:
  Stream* _serial;
  unsigned long _pollIntervalMs = 200;
  unsigned long _lastPoll = 0;
  unsigned int _receiveRetry = 25;

  char _pbuf[CMD_STR_MAX_LEN] = { 0 };
  char _method[CMD_METHOD_LEN_MAX] = { 0 };
};

#endif
