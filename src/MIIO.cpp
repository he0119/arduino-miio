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
      // https://iot.mi.com/v2/new/doc/resources-and-services/personal-developer/embedded-dev#MCU%20程序开发
      result |= ((UART_RECV_ACK_ERROR == sendStrWaitAck("echo off\r")) ? MIIO_ERROR : MIIO_OK);
      result |= ((UART_RECV_ACK_ERROR == sendStrWaitAck(String("model ") + String(model) + "\r")) ? MIIO_ERROR : MIIO_OK);
      result |= ((UART_RECV_ACK_ERROR == sendStrWaitAck(String("mcu_version ") + String(mcuVersion) + "\r")) ? MIIO_ERROR : MIIO_OK);
      result |= ((UART_RECV_ACK_ERROR == sendStrWaitAck(String("ble_config set ") + String(blePid) + String(" ") + String(mcuVersion) + "\r")) ? MIIO_ERROR : MIIO_OK);

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
  // TODO: 实现命令处理逻辑
  sendStr("hello world\n");
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

String MIIO::recvStr(unsigned long timeout)
{
  return _serial->readStringUntil(END_CHAR);
}

int MIIO::sendStr(const char* str)
{
  return _serial->write(str);
}

int MIIO::sendStr(String str)
{
  return sendStr(str.c_str());
}

int MIIO::sendStrWaitAck(const char* str)
{
  int len = strlen(str);
  if (len <= 0) { return UART_OK; }

  int n_send = _serial->write(str);

  String ack = recvStr(_serialTimeoutMs);

  if (ack != OK_STRING) {
    return UART_RECV_ACK_ERROR;
  }

  return n_send;
}

int MIIO::sendStrWaitAck(String str)
{
  return sendStrWaitAck(str.c_str());
}
