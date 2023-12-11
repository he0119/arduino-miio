/**
 * @author  MIoT,uy/sun
 * @date    2023
 * @par     Copyright (c):
 *
 *    Copyright 2023 uy/sun
 *    Copyright 2019 MIoT,MI
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "MIIO.h"

MIIO::MIIO(Stream &serial) {
  _serial = &serial;
}

void MIIO::begin(
    const char *model, const char *blePid, const char *mcuVersion) {
  {
    int result = 0;
    do {
      result = MIIO_OK;
      // https://iot.mi.com/v2/new/doc/resources-and-services/personal-developer/embedded-dev#MCU%20程序开发
      result |=
          ((UART_RECV_ACK_ERROR == sendStrWaitAck("echo off\r")) ? MIIO_ERROR
                                                                 : MIIO_OK);
      result |=
          ((UART_RECV_ACK_ERROR ==
            sendStrWaitAck(String("model ") + String(model) + "\r"))
               ? MIIO_ERROR
               : MIIO_OK);
      result |=
          ((UART_RECV_ACK_ERROR ==
            sendStrWaitAck(String("mcu_version ") + String(mcuVersion) + "\r"))
               ? MIIO_ERROR
               : MIIO_OK);
      result |=
          ((UART_RECV_ACK_ERROR ==
            sendStrWaitAck(
                String("ble_config set ") + String(blePid) + String(" ") +
                String(mcuVersion) + "\r"))
               ? MIIO_ERROR
               : MIIO_OK);

      delay(_pollIntervalMs);
    } while (result != MIIO_OK);
  }
}

void MIIO::begin(String model, String blePid, String mcuVersion) {
  begin(model.c_str(), blePid.c_str(), mcuVersion.c_str());
}

void MIIO::loop() {
  if (millis() - _lastPoll < _pollIntervalMs) {
    return;
  }
  _lastPoll = millis();

  /* clear command string buffer */
  memset(_pbuf, 0, CMD_STR_MAX_LEN);
  memset(_method, 0, CMD_METHOD_LEN_MAX);

  sendStr("get_down\r");

  // FIXME: 目前会卡住 loop，直到超时
  // 需要改成非阻塞的方式

  size_t nRecv = recvStr(_pbuf, CMD_STR_MAX_LEN);

  if (nRecv <= 0) {
    DEBUG_MIIO("[MIIO]uart connected error or module rebooting...");
    return;
  }

  if (_pbuf[nRecv - 1] != END_CHAR) {
    DEBUG_MIIO("[MIIO]uart recv error[%s]", _pbuf);
    return;
  }

  size_t methodLen = sizeof(_method);
  int ret;

  ret = uart_comamnd_decoder(_pbuf, nRecv, _method, &methodLen);
  if (MIIO_OK != ret) { /* judge if string decoded correctly */
    DEBUG_MIIO("[MIIO]get method failed[%s]", _pbuf);
    ret = MIIO_ERROR_PARAM;
    return;
  }

  if (methodLen > 0 &&
      _method != NULL) { /* start to find if method contained */
    auto callback = callbackFindByMethod(_method);
    if (NULL == callback) {
      if (strcmp(ERROR_STRING, _method) && strcmp(OK_STRING, _method)) {
        DEBUG_MIIO("[MIIO]undefined command: %s", _method);
      }
    } else {
      DEBUG_MIIO("[MIIO]found method: %s", _method);
      callback(_pbuf, nRecv);
    }
  } else {
    ret = MIIO_ERROR_PARAM;
    DEBUG_MIIO("[MIIO]unknown command: %s", (char *)_pbuf);
  }
}

void MIIO::setSerialTimeout(unsigned long timeout) {
  _serial->setTimeout(timeout);
}

void MIIO::setPollInterval(unsigned long interval) {
  _pollIntervalMs = interval;
}

void MIIO::setReceiveRetry(unsigned int retry) {
  _receiveRetry = retry;
}

size_t MIIO::sendStr(const char *str) {
  int len = strlen(str);
  if (len <= 0) {
    return UART_OK;
  }

  int nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[MIIO]send string: %s", str);

  return nSend;
}

size_t MIIO::sendStr(String str) {
  return sendStr(str.c_str());
}

size_t MIIO::sendStrWaitAck(const char *str) {
  size_t len = strlen(str);
  if (len <= 0) {
    return UART_OK;
  }

  size_t nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed 1");
    return UART_SEND_ERROR;
  }

  if (nSend < len) {
    DEBUG_MIIO("[MIIO]send string failed");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[MIIO]send string: %s", str);

  char ackBuf[ACK_BUF_SIZE] = {0};
  memset(ackBuf, 0, ACK_BUF_SIZE);

  recvStr(ackBuf, ACK_BUF_SIZE);

  if (0 != strncmp((const char *)ackBuf, "ok", strlen("ok"))) {
    DEBUG_MIIO("[MIIO]send string wait ack failed 2, str=%s", ackBuf);
    return UART_RECV_ACK_ERROR;
  }

  return nSend;
}

size_t MIIO::sendStrWaitAck(String str) {
  return sendStrWaitAck(str.c_str());
}

size_t MIIO::recvStr(char *buffer, size_t length) {
  int nRead = _serial->readBytes(buffer, length);

  int retry = 0;
  while (buffer[nRead > 0 ? (nRead - 1) : 0] != END_CHAR &&
         retry < _receiveRetry) {
    if (nRead >= length) {
      DEBUG_MIIO("[MIIO]out of buffer %d %d retry=%d", length, nRead, retry);
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
  } else {
    DEBUG_MIIO("[MIIO]recv string : null");
  }

  return nRead;
}

int MIIO::sendPropertyChanged(
    uint32_t siid, uint32_t piid, property_value_t *newValue) {
  int ret = 0;
  DEBUG_MIIO("[MIIO]=============== property changed ===============");

  property_operation_t *opt = NULL;

  if (NULL == newValue) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  do {
    opt = miio_property_operation_new();
    opt->code = 0;
    opt->siid = siid;
    opt->piid = piid;
    opt->value = newValue;
  } while (false);

  ret = executePropertyChanged(opt);

  if (ret != MIIO_OK) {
    DEBUG_MIIO("[MIIO]========= send property changed failed =========");
  } else {
    DEBUG_MIIO("[MIIO]======== send property changed success =========");
  }

  miio_property_operation_delete(opt);

  return ret;
}

int MIIO::executePropertyChanged(property_operation_t *opt) {
  int ret = MIIO_OK;

  char out[RESULT_BUF_SIZE] = {0};
  memset(out, 0, RESULT_BUF_SIZE);

  miio_changed_operation_encode(opt, out, RESULT_BUF_SIZE);
  int n_send = sendStr(out);
  if (n_send <= 0) {
    DEBUG_MIIO("[MIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  char res[RESULT_BUF_SIZE] = {0};
  memset(res, 0, RESULT_BUF_SIZE);

  int n_read = recvStr(res, RESULT_BUF_SIZE);
  if (n_read <= 0) {
    DEBUG_MIIO("[MIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  if (0 != strncmp(res, "ok", strlen("ok"))) {
    DEBUG_MIIO("[MIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  return ret;
}

int MIIO::onCommand(String method, MethodCallback callback) {
  if (method.isEmpty() || callback == NULL) {
    return MIIO_ERROR_PARAM;
  }

  _methodCallbacks[method] = callback;

  return MIIO_OK;
}

MethodCallback MIIO::callbackFindByMethod(const char *method) {
  if (method == NULL) {
    return NULL;
  }

  auto it = _methodCallbacks.find(method);
  if (it == _methodCallbacks.end()) {
    return NULL;
  }

  return it->second;
}
