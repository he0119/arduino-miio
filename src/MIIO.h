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


  /* 串口超时

  单位 ms
  */
  void setSerialTimeout(unsigned long timeout);

  /* 轮询间隔

  时间范围要求 100~200ms，推荐的循环周期为 200ms
  https://iot.mi.com/v2/new/doc/embedded-dev/module-dev/function-dev/mcu-dev#命令规范
  */
  void setPollInterval(unsigned long interval);

  String recvStr(unsigned long timeout = 0);

  int sendStr(const char* str);

  int sendStr(String str);

  int sendStrWaitAck(const char* str);

  int sendStrWaitAck(String str);


private:
  Stream* _serial;
  unsigned long _serialTimeoutMs = 200;
  unsigned long _pollIntervalMs = 200;
};

#endif
