#include "MIIO.h"
#include "miio/miio_define.h"

MIIO::MIIO(Stream& serial)
{
  _serial = &serial;
}

void MIIO::begin(const char* model, const char* blePid, const char* mcuVersion)
{
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
  if (millis() - _lastPoll < _pollIntervalMs) {
    return;
  }
  _lastPoll = millis();

  sendStr("get_down\r");

  String recv = recvStr();

  if (recv.isEmpty()) {
    return;
  }

  if (!recv.endsWith(String('\r'))) {
    return;
  }
}

void MIIO::setSerialTimeout(unsigned long timeout)
{
  _serial->setTimeout(timeout);
}

void MIIO::setPollInterval(unsigned long interval)
{
  _pollIntervalMs = interval;
}

String MIIO::recvStr()
{
  String recv = _serial->readStringUntil(END_CHAR);

  if (recv.isEmpty()) {
    DEBUG_MIIO("[MIIO]recv string: null\n");
  }
  else {
    DEBUG_MIIO("[MIIO]recv string: %s\n", recv);
  }

  return recv;
}

int MIIO::sendStr(const char* str)
{
  int len = strlen(str);
  if (len <= 0) { return UART_OK; }

  int nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed\n");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[MIIO]send string: %s\n", str);

  return nSend;
}

int MIIO::sendStr(String str)
{
  return sendStr(str.c_str());
}

int MIIO::sendStrWaitAck(const char* str)
{
  int len = strlen(str);
  if (len <= 0) { return UART_OK; }

  int nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed 1\n");
    return UART_SEND_ERROR;
  }

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed\n");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[MIIO]send string: %s\n", str);

  String ack = recvStr();

  if (ack != OK_STRING) {
    DEBUG_MIIO("[MIIO]send string wait ack failed 2, str=%s\n", ack);
    return UART_RECV_ACK_ERROR;
  }

  return nSend;
}

int MIIO::sendStrWaitAck(String str)
{
  return sendStrWaitAck(str.c_str());
}
