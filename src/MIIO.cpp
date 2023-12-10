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

  // FIXME: 目前会卡住 loop，直到超时
  // 需要改成非阻塞的方式
  size_t nRecv = recvStr(_pbuf, CMD_STR_MAX_LEN);

  if (nRecv <= 0) {
    DEBUG_MIIO("[MIIO]uart connected error or module rebooting...\n");
    return;
  }

  if (_pbuf[nRecv - 1] != END_CHAR) {
    DEBUG_MIIO("[MIIO]uart recv error[%s]\n", _pbuf);
    return;
  }

  // unsigned int methodLen = sizeof(_method);
  // int ret;

  // ret = uart_comamnd_decoder(_pbuf, nRecv, _method, &methodLen);
  // if (MIIO_OK != ret) { /* judge if string decoded correctly */
  //   DEBUG_MIIO("[MIIO]get method failed[%s]", _pbuf);
  //   ret = MIIO_ERROR_PARAM;
  //   return;
  // }
}

void MIIO::setSerialTimeout(unsigned long timeout)
{
  _serial->setTimeout(timeout);
}

void MIIO::setPollInterval(unsigned long interval)
{
  _pollIntervalMs = interval;
}


size_t MIIO::sendStr(const char* str)
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

size_t MIIO::sendStr(String str)
{
  return sendStr(str.c_str());
}

size_t MIIO::sendStrWaitAck(const char* str)
{
  size_t len = strlen(str);
  if (len <= 0) { return UART_OK; }

  size_t nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed 1\n");
    return UART_SEND_ERROR;
  }

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed\n");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[MIIO]send string: %s\n", str);

  char ackBuf[ACK_BUF_SIZE] = { 0 };
  memset(ackBuf, 0, ACK_BUF_SIZE);

  recvStr(ackBuf, ACK_BUF_SIZE);

  if (0 != strncmp((const char*)ackBuf, "ok", strlen("ok"))) {
    DEBUG_MIIO("[MIIO]send string wait ack failed 2, str=%s\n", ackBuf);
    return UART_RECV_ACK_ERROR;
  }

  return nSend;
}

size_t MIIO::sendStrWaitAck(String str)
{
  return sendStrWaitAck(str.c_str());
}

size_t MIIO::recvStr(char* buffer, size_t length)
{
  int nRead = _serial->readBytes(buffer, length);

  int retry = 0;
  while (buffer[nRead > 0 ? (nRead - 1) : 0] != END_CHAR && retry < _receiveRetry) {
    if (nRead >= length) {
      DEBUG_MIIO("[MIIO]out of buffer %d %d retry=%d\n", length, nRead, retry);
      memset(buffer, 0, length);
      nRead = 0;
      retry = 0;
    }
    nRead = nRead + _serial->readBytes(buffer + nRead, length - nRead);
    retry++;
  }

  buffer[nRead] = '\0';

  if (nRead > 0) {
    DEBUG_MIIO("[MIIO]recv string : %s", buffer);
  }
  else {
    DEBUG_MIIO("[MIIO]recv string : null");
  }

  return nRead;
}

void MIIO::setReceiveRetry(unsigned int retry)
{
  _receiveRetry = retry;
}
