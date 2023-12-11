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

  ret = uartComamndDecoder(_pbuf, nRecv, _method, &methodLen);
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

  if (NULL == newValue) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  property_operation_t opt = {
      .siid = 0, .piid = 0, .code = 0, .value = newValue};

  ret = executePropertyChanged(opt);

  if (ret != MIIO_OK) {
    DEBUG_MIIO("[MIIO]========= send property changed failed =========");
  } else {
    DEBUG_MIIO("[MIIO]======== send property changed success =========");
  }

  return ret;
}

int MIIO::executePropertyChanged(property_operation_t &opt) {
  int ret = MIIO_OK;

  char out[RESULT_BUF_SIZE] = {0};
  memset(out, 0, RESULT_BUF_SIZE);

  changedOperationEncode(opt, out, RESULT_BUF_SIZE);
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

int MIIO::uartComamndDecoder(
    char *pbuf, size_t buf_sz, char *method, size_t *methodLen) {
  char *temp = NULL;

  char *cmd_buf = (char *)malloc(buf_sz);
  if (NULL == cmd_buf || NULL == pbuf) {
    goto error_exit;
  }

  strncpy(cmd_buf, pbuf, buf_sz);
  cmd_buf[buf_sz - 1] = SPACE_CHAR;

  temp = strtok(cmd_buf, SPACE_STRING); /* pass "down" */
  if (NULL == temp) {
    goto error_exit;
  }

  /* judge if recieved "error\r" */
  if (!strncmp(
          temp,
          ERROR_STRING,
          strlen(ERROR_STRING) < strlen(temp) ? strlen(ERROR_STRING)
                                              : strlen(temp))) {
    goto error_exit;
  } else if (NULL == temp) {
    goto error_exit;
  }

  /* get "get_properties"/"set_properties"/"none"/"update_fw"/"action" */
  temp = strtok(NULL, SPACE_STRING);
  if (NULL == temp) {
    goto error_exit;
  }

normal_exit: {
  strncpy(method, temp, strlen(temp));
  *methodLen = strlen(method);

  if (NULL != cmd_buf) {
    free(cmd_buf);
  }
  return MIIO_OK;
}

error_exit: {
  method = NULL;
  *methodLen = 0;
  if (NULL != cmd_buf) {
    free(cmd_buf);
  }

  return MIIO_ERROR_PARAM;
}
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

int MIIO::changedOperationEncodeEnd(char out[], size_t size) {
  int ret = MIIO_OK;

  if (strlen(out) > size - 1) {
    ret = MIIO_ERROR;
    return ret;
  }

  str_n_cat(out, 1, "\r");

  return ret;
}

int MIIO::changedOperationEncode(
    property_operation_t &opt, char out[], size_t size) {
  memset(out, 0, size);
  str_n_cat(out, 2, "properties_changed", " ");

  char siid_buf[ID_MAX_LEN] = {0};
  char piid_buf[ID_MAX_LEN] = {0};

  snprintf(siid_buf, ID_MAX_LEN, "%d", opt.siid);
  snprintf(piid_buf, ID_MAX_LEN, "%d", opt.piid);

  str_n_cat(out, 4, siid_buf, " ", piid_buf, " ");

  switch (opt.value->format) {
  case PROPERTY_FORMAT_BOOLEAN: {
    if (opt.value->data.boolean.value == false) {
      str_n_cat(out, 2, "false", " ");
    } else {
      str_n_cat(out, 2, "true", " ");
    }
  } break;

  case PROPERTY_FORMAT_STRING: {
    str_n_cat(out, 2, opt.value->data.string.value, " ");
  } break;

  case PROPERTY_FORMAT_NUMBER: {
    if (opt.value->data.number.type == DATA_NUMBER_INTEGER) {
      char integer_buf[VALUE_MAX_LEN] = {0};
      snprintf(
          integer_buf,
          VALUE_MAX_LEN,
          "%d",
          (int)opt.value->data.number.value.integerValue);
      str_n_cat(out, 2, integer_buf, " ");
    } else if (opt.value->data.number.type == DATA_NUMBER_FLOAT) {
      char float_buf[VALUE_MAX_LEN] = {0};
      snprintf(
          float_buf,
          VALUE_MAX_LEN,
          "%f",
          opt.value->data.number.value.floatValue);
      str_n_cat(out, 2, float_buf, " ");
    }
  } break;

  case PROPERTY_FORMAT_UNDEFINED:
    break;
  }

  changedOperationEncodeEnd(out, size);

  return MIIO_OK;
}
