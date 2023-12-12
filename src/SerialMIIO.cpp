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
#include "SerialMIIO.h"

SerialMIIO::SerialMIIO(Stream &serial) {
  _serial = &serial;

  // 注册默认的回调函数
  // 用于处理 get_properties/set_properties/action/none/mcu_version_req
  // 可以被用户覆盖
  onMethod(
      GET_PRO_STRING,
      std::bind(
          &SerialMIIO::_defaultGetPropertiesCallback,
          this,
          std::placeholders::_1,
          std::placeholders::_2));
  onMethod(
      SET_PRO_STRING,
      std::bind(
          &SerialMIIO::_defaultSetPropertyCallback,
          this,
          std::placeholders::_1,
          std::placeholders::_2));
  onMethod(
      ACTION_STRING,
      std::bind(
          &SerialMIIO::_defaultinvokeActionCallback,
          this,
          std::placeholders::_1,
          std::placeholders::_2));
  onMethod(
      NONE_STRING,
      std::bind(
          &SerialMIIO::_defaultinvokeNoneCallback,
          this,
          std::placeholders::_1,
          std::placeholders::_2));

  onMethod(
      MCU_VERSION_REQ_STRING,
      std::bind(
          &SerialMIIO::_defaultMCUVersionCallback,
          this,
          std::placeholders::_1,
          std::placeholders::_2));
}

void SerialMIIO::begin(
    const char *model, const char *blePid, const char *mcuVersion) {
  {
    _mcuVersion = mcuVersion;

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

void SerialMIIO::begin(String model, String blePid, String mcuVersion) {
  begin(model.c_str(), blePid.c_str(), mcuVersion.c_str());
}

void SerialMIIO::loop() {
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
    DEBUG_MIIO("[SerialMIIO]uart connected error or module rebooting...");
    return;
  }

  if (_pbuf[nRecv - 1] != END_CHAR) {
    DEBUG_MIIO("[SerialMIIO]uart recv error[%s]", _pbuf);
    return;
  }

  size_t methodLen = sizeof(_method);
  int ret;

  ret = uart_comamnd_decoder(_pbuf, nRecv, _method, &methodLen);
  if (MIIO_OK != ret) { /* judge if string decoded correctly */
    DEBUG_MIIO("[SerialMIIO]get method failed[%s]", _pbuf);
    ret = MIIO_ERROR_PARAM;
    return;
  }

  if (methodLen > 0 &&
      _method != NULL) { /* start to find if method contained */
    auto callback = callbackFindByMethod(_method);
    if (NULL == callback) {
      if (strcmp(ERROR_STRING, _method) && strcmp(OK_STRING, _method)) {
        sendErrorCode(ERROR_MESSAGE_UNCMD, ERROR_CODE_UNCMD);
        DEBUG_MIIO("[SerialMIIO]undefined command: %s", _method);
      }
    } else {
      DEBUG_MIIO("[SerialMIIO]found method: %s", _method);
      callback(_pbuf, nRecv);
    }
  } else {
    ret = MIIO_ERROR_PARAM;
    DEBUG_MIIO("[SerialMIIO]unknown command: %s", (char *)_pbuf);
  }
}

void SerialMIIO::setSerialTimeout(unsigned long timeout) {
  _serialTimeout = timeout;
}

void SerialMIIO::setPollInterval(unsigned long interval) {
  _pollIntervalMs = interval;
}

void SerialMIIO::setReceiveRetry(unsigned int retry) {
  _receiveRetry = retry;
}

size_t SerialMIIO::sendStr(const char *str) {
  int len = strlen(str);
  if (len <= 0) {
    return UART_OK;
  }

  int nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[SerialMIIO]send string failed");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[SerialMIIO]send string: %s", str);

  return nSend;
}

size_t SerialMIIO::sendStr(String str) {
  return sendStr(str.c_str());
}

size_t SerialMIIO::sendStrWaitAck(const char *str) {
  size_t len = strlen(str);
  if (len <= 0) {
    return UART_OK;
  }

  size_t nSend = _serial->write(str);

  if (nSend < len) {
    DEBUG_MIIO("[SerialMIIO]send string failed 1");
    return UART_SEND_ERROR;
  }

  if (nSend < len) {
    DEBUG_MIIO("[SerialMIIO]send string failed");
    return UART_SEND_ERROR;
  }

  DEBUG_MIIO("[SerialMIIO]send string: %s", str);

  char ackBuf[ACK_BUF_SIZE] = {0};
  memset(ackBuf, 0, ACK_BUF_SIZE);

  recvStr(ackBuf, ACK_BUF_SIZE);

  if (0 != strncmp((const char *)ackBuf, "ok", strlen("ok"))) {
    DEBUG_MIIO("[SerialMIIO]send string wait ack failed 2, str=%s", ackBuf);
    return UART_RECV_ACK_ERROR;
  }

  return nSend;
}

size_t SerialMIIO::sendStrWaitAck(String str) {
  return sendStrWaitAck(str.c_str());
}

size_t SerialMIIO::recvStr(char *buffer, size_t length) {
  int nRead = _readBytes(buffer, length);

  int retry = 0;
  while (buffer[nRead > 0 ? (nRead - 1) : 0] != END_CHAR &&
         retry < _receiveRetry) {
    if (nRead >= length) {
      DEBUG_MIIO(
          "[SerialMIIO]out of buffer %d %d retry=%d", length, nRead, retry);
      memset(buffer, 0, length);
      nRead = 0;
      retry = 0;
    }
    nRead = nRead + _readBytes(buffer + nRead, length - nRead);
    retry++;
  }

  buffer[nRead] = '\0';

  if (nRead > 0) {
    DEBUG_MIIO("[SerialMIIO]recv string : %s", buffer);
  } else {
    DEBUG_MIIO("[SerialMIIO]recv string : null");
  }

  return nRead;
}

int SerialMIIO::sendPropertyChanged(
    uint32_t siid, uint32_t piid, property_value_t *newValue) {
  int ret = 0;
  DEBUG_MIIO("[SerialMIIO]=============== property changed ===============");

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
    DEBUG_MIIO("[SerialMIIO]========= send property changed failed =========");
  } else {
    DEBUG_MIIO("[SerialMIIO]======== send property changed success =========");
  }

  miio_property_operation_delete(opt);

  return ret;
}

int SerialMIIO::executePropertyChanged(property_operation_t *opt) {
  int ret = MIIO_OK;

  char out[RESULT_BUF_SIZE] = {0};
  memset(out, 0, RESULT_BUF_SIZE);

  miio_changed_operation_encode(opt, out, RESULT_BUF_SIZE);
  int n_send = sendStr(out);
  if (n_send <= 0) {
    DEBUG_MIIO("[SerialMIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  char res[RESULT_BUF_SIZE] = {0};
  memset(res, 0, RESULT_BUF_SIZE);

  int n_read = recvStr(res, RESULT_BUF_SIZE);
  if (n_read <= 0) {
    DEBUG_MIIO("[SerialMIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  if (0 != strncmp(res, "ok", strlen("ok"))) {
    DEBUG_MIIO("[SerialMIIO]property changed send failed");
    ret = MIIO_ERROR;
    return ret;
  }

  return ret;
}

void SerialMIIO::onMethod(String method, MethodCallback callback) {
  if (method.isEmpty() || callback == NULL) {
    return;
  }

  _methodCallbacks[method] = callback;
}

void SerialMIIO::onActionInvoke(
    uint32_t siid, uint32_t aiid, ActionInvokeCallback callback) {
  if (callback == NULL) {
    return;
  }

  _actionInvokeCallbacks[{siid, aiid}] = callback;
}

void SerialMIIO::onPropertyGet(
    uint32_t siid, uint32_t piid, PropertyCallback callback) {
  if (callback == NULL) {
    return;
  }

  _propertyGetCallbacks[{siid, piid}] = callback;
}

void SerialMIIO::onPropertySet(
    uint32_t siid, uint32_t piid, PropertyCallback callback) {
  if (callback == NULL) {
    return;
  }

  _propertySetCallbacks[{siid, piid}] = callback;
}

ActionInvokeCallback
SerialMIIO::callbackFindByAction(uint32_t siid, uint32_t aiid) {
  auto it = _actionInvokeCallbacks.find({siid, aiid});
  if (it == _actionInvokeCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

PropertyCallback
SerialMIIO::callbackFindByPropertyGet(uint32_t siid, uint32_t piid) {
  auto it = _propertyGetCallbacks.find({siid, piid});
  if (it == _propertyGetCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

PropertyCallback
SerialMIIO::callbackFindByPropertySet(uint32_t siid, uint32_t piid) {
  auto it = _propertySetCallbacks.find({siid, piid});
  if (it == _propertySetCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

MethodCallback SerialMIIO::callbackFindByMethod(const char *method) {
  if (method == NULL) {
    return NULL;
  }

  auto it = _methodCallbacks.find(method);
  if (it == _methodCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

int SerialMIIO::sendEventOccurred(event_operation_t *event) {
  int ret = MIIO_OK;

  DEBUG_MIIO("[SerialMIIO]================= event occurred ===============");

  if (NULL == event) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  do {
    char out[RESULT_BUF_SIZE] = {0};
    memset(out, 0, RESULT_BUF_SIZE);

    miio_event_operation_encode(event, out, RESULT_BUF_SIZE);

    ret = eventSend(out, strlen(out));
  } while (false);

  if (MIIO_OK == ret) {
    DEBUG_MIIO("[SerialMIIO]============== event send success ==============");
  } else {
    DEBUG_MIIO("[SerialMIIO]============== event send failed  ==============");
  }

  return ret;
}

int SerialMIIO::eventSend(char out[], size_t len) {
  int ret = MIIO_OK;
  char res[RESULT_BUF_SIZE] = {0};
  memset(res, 0, RESULT_BUF_SIZE);

  int n_send = sendStr(out);
  if (n_send < 0) return MIIO_ERROR;
  if (n_send != len) {
    DEBUG_MIIO("[SerialMIIO]send event failed 1");
    ret = MIIO_ERROR;
    return ret;
  }

  int n_read = recvStr(res, RESULT_BUF_SIZE);
  if (n_read <= 0) {
    DEBUG_MIIO("[SerialMIIO]send event failed 2");
    ret = MIIO_ERROR;
    return ret;
  }

  if (0 != strncmp(res, "ok", strlen("ok"))) {
    DEBUG_MIIO("[SerialMIIO]send event failed 3");
    ret = MIIO_ERROR;
    return ret;
  }

  return ret;
}

int SerialMIIO::executePropertyOperation(
    const char *pbuf, int buf_sz, property_operation_type type) {
  int ret = 0;

  char *cmd_buf = (char *)calloc(1, buf_sz);
  if (NULL == cmd_buf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmd_buf, pbuf, buf_sz - 1);
  cmd_buf[buf_sz - 1] = '\0'; /* remove '\r' at the end */

  int params_pairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmd_buf, " "); /* pass string "down" */
  temp = strtok(NULL, " "); /* pass string "get_properties"/"set_properties" */

  while (1) {
    temp = strtok(NULL, " ");
    if (NULL != temp) {
      params_pairs++;
    } else {
      break;
    }
  }

  switch (type) {

  case PROPERTY_OPERATION_GET: {
    if (params_pairs % 2 != 0) {
      DEBUG_MIIO("[SerialMIIO]params error");
      ret = -1;
    } else {
      params_pairs /= 2;
    }
  } break;

  case PROPERTY_OPERATION_SET: {
    if (params_pairs % 3 != 0) {
      DEBUG_MIIO("[SerialMIIO]params error");
      ret = -1;
    } else {
      params_pairs /= 3;
    }
  } break;
  }

  do {
    bool has_value = (type == PROPERTY_OPERATION_SET);

    char result[RESULT_BUF_SIZE] = {0};
    str_n_cat(result, 1, "result ");

    for (uint32_t i = 0; i < params_pairs; ++i) {
      if (RESULT_MAX_LEN < strlen(result)) {
        ret = MIIO_ERROR_SIZE;
        break;
      }
      /* decode command params */
      property_operation_t *opt =
          miio_property_operation_decode(pbuf, buf_sz, i, has_value);

      if (opt == NULL) {
        break;
      }

      if (has_value) {
        _onPropertySet(opt);
      } else {
        _onPropertyGet(opt);
      }

      property_operation_encode_param(result, RESULT_BUF_SIZE, opt, has_value);
      miio_property_operation_delete(opt);
    }

    if (MIIO_ERROR_SIZE == ret) {
      DEBUG_MIIO("[SerialMIIO]result too long");
      sendErrorCode(ERROR_MESSAGE_RESULT_TOO_LONG, ERROR_CODE_RESULT_TOO_LONG);
      break;
    }

    property_operation_encode_tail(result, RESULT_BUF_SIZE);
    sendResponse(result);
  } while (false);

  if (NULL != cmd_buf) {
    free(cmd_buf);
  }

  return ret;
}

int SerialMIIO::executeActionInvocation(const char *pbuf, int buf_sz) {
  int ret = 0;

  char *cmd_buf = (char *)calloc(1, buf_sz);
  if (NULL == cmd_buf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmd_buf, pbuf, buf_sz - 1);
  cmd_buf[buf_sz - 1] = '\0'; /* remove '\r' at the end */
  int params_pairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmd_buf, " "); /* pass string "down" */
  temp = strtok(NULL, " ");          /* pass string "action" */

  while (1) {
    temp = strtok(NULL, " ");
    if (NULL != temp) {
      params_pairs++;
    } else {
      break;
    }
  }

  if (params_pairs >= 4 && params_pairs % 2 != 0) {
    DEBUG_MIIO("[SerialMIIO]params error");
    ret = -1;
    sendErrorCode(ERROR_MESSAGE_UNPARAMS, ERROR_CODE_UNPARAMS);
    return ret;
  } else {
    params_pairs = (params_pairs + 3) / 4;
  }

  do {
    char result[RESULT_BUF_SIZE] = {0};
    str_n_cat(result, 1, "result ");

    for (uint32_t i = 0; i < params_pairs; ++i) {

      // decode command params
      action_operation_t *opt = action_operation_decode(pbuf, buf_sz, i);

      if (opt == NULL) {
        break;
      }

      _onActionInvoke(opt);
      action_operation_encode_param(result, sizeof(result), opt);
      action_operation_delete(opt);
    }

    action_operation_encode_tail(result, sizeof(result));
    sendResponse(result);
  } while (false);

  if (NULL != cmd_buf) {
    free(cmd_buf);
  }

  return ret;
}

int SerialMIIO::sendResponse(const char *pbuf) {
  int ret = MIIO_OK;

  size_t buf_sz = strlen(pbuf);

  if (NULL == pbuf || buf_sz <= 0) {
    DEBUG_MIIO("[SerialMIIO]handle or pbuf is null");
    return MIIO_ERROR;
  }

  int n_send = sendStrWaitAck(pbuf);

  if (n_send < 0) return MIIO_ERROR;

  if (n_send < buf_sz) {
    DEBUG_MIIO("[SerialMIIO]uart send result failed");
    ret = MIIO_ERROR;
  }
  return ret;
}

int SerialMIIO::sendErrorCode(const char *pbuf, int errcode) {
  char send_msg[RESULT_BUF_SIZE] = {0};
  memset(send_msg, 0, RESULT_BUF_SIZE);
  snprintf(send_msg, RESULT_BUF_SIZE, "\"%s\" %d", pbuf, errcode);

  int ret = 0;
  do {
    char result[RESULT_BUF_SIZE] = {0};
    str_n_cat(result, 2, "error ", send_msg);
    action_operation_encode_tail(result, sizeof(result));
    sendResponse(result);
  } while (false);

  return ret;

  return MIIO_OK;
}

void SerialMIIO::_onActionInvoke(action_operation_t *o) {
  DEBUG_MIIO("[SerialMIIO]on_action_invoke");
  DEBUG_MIIO("[SerialMIIO]siid: %d", o->siid);
  DEBUG_MIIO("[SerialMIIO]aiid: %d", o->aiid);

  auto callback = callbackFindByAction(o->siid, o->aiid);
  if (NULL == callback) {
    DEBUG_MIIO("[SerialMIIO]undefined action: %d %d", o->siid, o->aiid);
    o->code = OPERATION_INVALID;
    return;
  }

  callback(o);
}

void SerialMIIO::_onPropertyGet(property_operation_t *o) {
  DEBUG_MIIO("[SerialMIIO]on_property_get");
  DEBUG_MIIO("[SerialMIIO]siid: %d", o->siid);
  DEBUG_MIIO("[SerialMIIO]piid: %d", o->piid);

  auto callback = callbackFindByPropertyGet(o->siid, o->piid);
  if (NULL == callback) {
    DEBUG_MIIO("[SerialMIIO]undefined property get: %d %d", o->siid, o->piid);
    o->code = OPERATION_INVALID;
    return;
  }

  callback(o);
}

void SerialMIIO::_onPropertySet(property_operation_t *o) {
  DEBUG_MIIO("[SerialMIIO]on_property_set");
  DEBUG_MIIO("[SerialMIIO]siid: %d", o->siid);
  DEBUG_MIIO("[SerialMIIO]piid: %d", o->piid);

  auto callback = callbackFindByPropertySet(o->siid, o->piid);
  if (NULL == callback) {
    DEBUG_MIIO("[SerialMIIO]undefined property set: %d %d", o->siid, o->piid);
    o->code = OPERATION_INVALID;
    return;
  }

  callback(o);
}

void SerialMIIO::_defaultGetPropertiesCallback(char *cmd, size_t length) {
  DEBUG_MIIO(
      "[SerialMIIO]==================== get_properties ====================");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_GET);
  ;
}

void SerialMIIO::_defaultSetPropertyCallback(char *cmd, size_t length) {
  DEBUG_MIIO(
      "[SerialMIIO]==================== set_properties ====================");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_SET);
}

void SerialMIIO::_defaultinvokeActionCallback(char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]==================== action ====================");
  executeActionInvocation(cmd, length);
}

void SerialMIIO::_defaultinvokeNoneCallback(char *cmd, size_t length) {
  DEBUG_MIIO(
      "[SerialMIIO]========================= none ==========================");
}

void SerialMIIO::_defaultMCUVersionCallback(char *cmd, size_t length) {
  DEBUG_MIIO(
      "[SerialMIIO]==================== mcu_version_req ====================");

  char result[RESULT_BUF_SIZE] = {0};
  str_n_cat(result, 2, "mcu_version ", _mcuVersion);
  action_operation_encode_tail(result, sizeof(result));
  sendResponse(result);
}

size_t SerialMIIO::_readBytes(char *buffer, size_t length) {
  size_t count = 0;
  while (count < length) {
    int c = _timedRead();
    if (c < 0) {
      break;
    }
    *buffer++ = (char)c;
    count++;
  }
  return count;
}

int SerialMIIO::_timedRead() {
  int c;
  _serialStartMillis = millis();
  do {
    c = _serial->read();
    if (c >= 0) {
      return c;
    }
    // 如果没有读取到数据，则交出控制权
    vTaskDelay(1);
  } while (millis() - _serialStartMillis < _serialTimeout);
  return -1; // -1 indicates timeout
}
