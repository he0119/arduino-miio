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

  _cmd.reserve(CMD_BUF_SIZE);

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
  begin(String(model), String(blePid), String(mcuVersion));
}

void SerialMIIO::begin(String model, String blePid, String mcuVersion) {
  _model = model;
  _blePid = blePid;
  _mcuVersion = mcuVersion;

  _handleXiaomiSetup(false);
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

size_t SerialMIIO::sendStr(const String &str, ReceiveCallback callback) {
  _receiveCallback = callback;

  if (str.isEmpty()) {
    return UART_OK;
  }
  size_t length = str.length();

  size_t nSend = _serial->print(str);
  // 如果字符串没有以 \r 结尾，则补上
  if (!str.endsWith("\r")) {
    nSend += _serial->print("\r");
    length += 1;
  }

  DEBUG_MIIO("[SerialMIIO]send string: %s", str.c_str());

  if (nSend < str.length()) {
    DEBUG_MIIO("[SerialMIIO]send string failed");
    return UART_SEND_ERROR;
  }

  return nSend;
}

size_t SerialMIIO::sendStr(const char *str, ReceiveCallback callback) {
  return sendStr(String(str), callback);
}

size_t SerialMIIO::sendStrWaitAck(const String &str) {
  if (str.isEmpty()) {
    return UART_OK;
  }

  size_t nSend = sendStr(
      str, std::bind(&SerialMIIO::_handleAck, this, std::placeholders::_1));

  return nSend;
}

size_t SerialMIIO::sendStrWaitAck(const char *str) {
  return sendStrWaitAck(String(str));
}

size_t
SerialMIIO::sendStrWaitAck(const String &str, AckResultCallback callback) {
  _ackResultCallback = callback;
  return sendStrWaitAck(str);
}

size_t SerialMIIO::sendStrWaitAck(const char *str, AckResultCallback callback) {
  _ackResultCallback = callback;
  return sendStrWaitAck(str);
}

int SerialMIIO::sendResponse(const String &response) {
  int ret = MIIO_OK;

  if (response.isEmpty()) {
    DEBUG_MIIO("[SerialMIIO]response is empty");
    return MIIO_ERROR;
  }

  int nSend = sendStrWaitAck(response);

  if (nSend < 0) {
    DEBUG_MIIO("[SerialMIIO]uart send result failed");
    return MIIO_ERROR;
  }

  if (nSend < response.length()) {
    DEBUG_MIIO("[SerialMIIO]uart send result incomplete");
    ret = MIIO_ERROR;
  }
  return ret;
}

int SerialMIIO::sendResponse(const char *response) {
  return sendResponse(String(response));
}

int SerialMIIO::sendErrorCode(const String &msg, int errcode) {
  String result;
  result.reserve(CMD_BUF_SIZE);

  result += "error ";
  result += "\"";
  result += msg;
  result += "\" ";
  result += errcode;

  return sendResponse(result);
}

int SerialMIIO::sendErrorCode(const char *msg, int errcode) {
  return sendErrorCode(String(msg), errcode);
}

int SerialMIIO::sendPropertyChanged(
    uint32_t siid, uint32_t piid, property_value_t *newValue) {
  DEBUG_MIIO("[SerialMIIO]property changed");
  int ret = MIIO_OK;

  if (NULL == newValue) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  property_operation_t *opt = NULL;
  opt = miio_property_operation_new();
  opt->code = 0;
  opt->siid = siid;
  opt->piid = piid;
  opt->value = newValue;

  char out[CMD_BUF_SIZE] = {0};
  memset(out, 0, CMD_BUF_SIZE);

  miio_changed_operation_encode(opt, out, CMD_BUF_SIZE);

  ret = sendResponse(out);

  if (ret != MIIO_OK) {
    DEBUG_MIIO("[SerialMIIO]send property changed failed");
  } else {
    DEBUG_MIIO("[SerialMIIO]send property changed success");
  }

  miio_property_operation_delete(opt);

  return ret;
}

int SerialMIIO::sendEventOccurred(event_operation_t *event) {
  DEBUG_MIIO("[SerialMIIO]event occurred");

  int ret = MIIO_OK;

  if (NULL == event) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  char out[CMD_BUF_SIZE] = {0};
  memset(out, 0, CMD_BUF_SIZE);

  miio_event_operation_encode(event, out, CMD_BUF_SIZE);

  ret = sendResponse(out);

  if (MIIO_OK == ret) {
    DEBUG_MIIO("[SerialMIIO]event send success");
  } else {
    DEBUG_MIIO("[SerialMIIO]event send failed");
  }

  return ret;
}

int SerialMIIO::executePropertyOperation(
    const char *cmd, size_t length, property_operation_type type) {
  int ret = MIIO_OK;

  char *cmdBuf = (char *)calloc(1, length);
  if (NULL == cmdBuf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmdBuf, cmd, length - 1);
  cmdBuf[length - 1] = '\0'; /* remove '\r' at the end */

  int paramsPairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmdBuf, " "); /* pass string "down" */
  temp = strtok(NULL, " "); /* pass string "get_properties"/"set_properties" */

  while (1) {
    temp = strtok(NULL, " ");
    if (NULL != temp) {
      paramsPairs++;
    } else {
      break;
    }
  }

  switch (type) {
  case PROPERTY_OPERATION_GET: {
    if (paramsPairs % 2 != 0) {
      DEBUG_MIIO("[SerialMIIO]params error");
      ret = MIIO_ERROR_PARAM;
    } else {
      paramsPairs /= 2;
    }
  } break;

  case PROPERTY_OPERATION_SET: {
    if (paramsPairs % 3 != 0) {
      DEBUG_MIIO("[SerialMIIO]params error");
      ret = MIIO_ERROR_PARAM;
    } else {
      paramsPairs /= 3;
    }
  } break;
  }

  do {
    if (MIIO_ERROR_PARAM == ret) {
      sendErrorCode(ERROR_MESSAGE_UNPARAMS, ERROR_CODE_UNPARAMS);
      break;
    }

    bool hasValue = (type == PROPERTY_OPERATION_SET);

    char result[CMD_BUF_SIZE] = {0};
    str_n_cat(result, 1, "result ");

    for (uint32_t i = 0; i < paramsPairs; ++i) {
      if (CMD_MAX_LEN < strlen(result)) {
        ret = MIIO_ERROR_SIZE;
        break;
      }
      /* decode command params */
      property_operation_t *opt =
          miio_property_operation_decode(cmd, length, i, hasValue);

      if (opt == NULL) {
        break;
      }

      if (hasValue) {
        _onPropertySet(opt);
      } else {
        _onPropertyGet(opt);
      }

      property_operation_encode_param(result, CMD_BUF_SIZE, opt, hasValue);
      miio_property_operation_delete(opt);
    }

    if (MIIO_ERROR_SIZE == ret) {
      DEBUG_MIIO("[SerialMIIO]result too long");
      sendErrorCode(ERROR_MESSAGE_RESULT_TOO_LONG, ERROR_CODE_RESULT_TOO_LONG);
      break;
    }

    property_operation_encode_tail(result, CMD_BUF_SIZE);
    sendResponse(result);
  } while (false);

  if (NULL != cmdBuf) {
    free(cmdBuf);
  }

  return ret;
}

int SerialMIIO::executeActionInvocation(const char *cmd, size_t length) {
  int ret = MIIO_OK;

  char *cmdBuf = (char *)calloc(1, length);
  if (NULL == cmdBuf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmdBuf, cmd, length - 1);
  cmdBuf[length - 1] = '\0'; /* remove '\r' at the end */
  int paramsPairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmdBuf, " "); /* pass string "down" */
  temp = strtok(NULL, " ");         /* pass string "action" */

  while (1) {
    temp = strtok(NULL, " ");
    if (NULL != temp) {
      paramsPairs++;
    } else {
      break;
    }
  }

  if (paramsPairs >= 4 && paramsPairs % 2 != 0) {
    DEBUG_MIIO("[SerialMIIO]params error");
    ret = MIIO_ERROR_PARAM;
  } else {
    paramsPairs = (paramsPairs + 3) / 4;
  }

  do {
    if (MIIO_ERROR_PARAM == ret) {
      sendErrorCode(ERROR_MESSAGE_UNPARAMS, ERROR_CODE_UNPARAMS);
      break;
    }

    char result[CMD_BUF_SIZE] = {0};
    str_n_cat(result, 1, "result ");

    for (uint32_t i = 0; i < paramsPairs; ++i) {
      if (CMD_MAX_LEN < strlen(result)) {
        ret = MIIO_ERROR_SIZE;
        break;
      }
      // decode command params
      action_operation_t *opt = action_operation_decode(cmd, length, i);

      if (opt == NULL) {
        break;
      }

      _onActionInvoke(opt);
      action_operation_encode_param(result, sizeof(result), opt);
      action_operation_delete(opt);
    }

    if (MIIO_ERROR_SIZE == ret) {
      DEBUG_MIIO("[SerialMIIO]result too long");
      sendErrorCode(ERROR_MESSAGE_RESULT_TOO_LONG, ERROR_CODE_RESULT_TOO_LONG);
      break;
    }

    action_operation_encode_tail(result, sizeof(result));
    sendResponse(result);
  } while (false);

  if (NULL != cmdBuf) {
    free(cmdBuf);
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

SerialMIIO::ActionInvokeCallback
SerialMIIO::callbackFindByAction(uint32_t siid, uint32_t aiid) {
  auto it = _actionInvokeCallbacks.find({siid, aiid});
  if (it == _actionInvokeCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

SerialMIIO::PropertyCallback
SerialMIIO::callbackFindByPropertyGet(uint32_t siid, uint32_t piid) {
  auto it = _propertyGetCallbacks.find({siid, piid});
  if (it == _propertyGetCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

SerialMIIO::PropertyCallback
SerialMIIO::callbackFindByPropertySet(uint32_t siid, uint32_t piid) {
  auto it = _propertySetCallbacks.find({siid, piid});
  if (it == _propertySetCallbacks.end()) {
    return NULL;
  }

  return it->second;
}

SerialMIIO::MethodCallback
SerialMIIO::callbackFindByMethod(const char *method) {
  if (method == NULL) {
    return NULL;
  }

  auto it = _methodCallbacks.find(method);
  if (it == _methodCallbacks.end()) {
    return NULL;
  }

  return it->second;
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

void SerialMIIO::_defaultGetPropertiesCallback(const char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]down get_properties default callback");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_GET);
}

void SerialMIIO::_defaultSetPropertyCallback(const char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]down set_properties default callback");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_SET);
}

void SerialMIIO::_defaultinvokeActionCallback(const char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]down action default callback");

  executeActionInvocation(cmd, length);
}

void SerialMIIO::_defaultinvokeNoneCallback(const char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]down none default callback");
}

void SerialMIIO::_defaultMCUVersionCallback(const char *cmd, size_t length) {
  DEBUG_MIIO("[SerialMIIO]down mcu_version_req default callback");

  char result[CMD_BUF_SIZE] = {0};
  str_n_cat(result, 2, "mcu_version ", _mcuVersion);
  action_operation_encode_tail(result, sizeof(result));
  sendResponse(result);
}

void SerialMIIO::loop() {
  _sendGetDown();
  _read();
}

void SerialMIIO::_sendGetDown() {
  // 没有初始化成功
  if (_setupStatus != SETUP_OK) {
    if (millis() - _lastPoll > _pollIntervalMs && NULL == _receiveCallback) {
      _lastPoll = millis();
      _handleXiaomiSetup(false);
    }
    return;
  }
  // 没有回调函数
  if (NULL != _receiveCallback) {
    return;
  }
  // 需要发送 get_down
  if (!_needGetDown) {
    return;
  }
  // 没有达到轮询时间
  if (millis() - _lastPoll < _pollIntervalMs && _lastPoll != 0) {
    return;
  }
  // 以上情况均不发送 get_down
  _lastPoll = millis();
  _cmd.clear();
  _needGetDown = false;

  sendStr(
      GET_DOWN_STRING,
      std::bind(&SerialMIIO::_handleGetDown, this, std::placeholders::_1));
}

void SerialMIIO::_read() {
  // 超时时增加重试次数
  if (_serial->available() <= 0 &&
      millis() - _serialStartMillis > _serialTimeout) {
    // DEBUG_MIIO("[SerialMIIO]read timeout");
    _retry++;
    _serialStartMillis = millis();
    return;
  }

  while (_serial->available() > 0) {
    _cmd += (char)_serial->read();
    _serialStartMillis = millis();

    if (_cmd.length() > CMD_BUF_SIZE) {
      DEBUG_MIIO("[SerialMIIO]cmd too long %d retry=%d", _cmd.length(), _retry);
      _cmd.clear();
      _retry = 0;
      continue;
    }

    if (_cmd.endsWith(END_STRING)) {
      DEBUG_MIIO("[SerialMIIO]cmd end");
      _executeReceiveCallbacks(_cmd);
      return;
    }
  }

  if (_retry > _receiveRetry) {
    DEBUG_MIIO("[SerialMIIO]read retry too many times");
    _executeReceiveCallbacks(_cmd);
    return;
  }
}

void SerialMIIO::_executeReceiveCallbacks(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]execute receive callbacks");

  if (NULL == _receiveCallback) {
    DEBUG_MIIO("[SerialMIIO]no receive callback");
  } else {
    auto callback = _receiveCallback;
    _receiveCallback = NULL;
    callback(cmd);
  }

  // 无论什么时候，执行完回调后，都清空缓存
  _cmd.clear();
  _retry = 0;
}

void SerialMIIO::_executeackResultCallbacks(bool result) {
  DEBUG_MIIO("[SerialMIIO]execute ack result callbacks");

  if (NULL == _ackResultCallback) {
    DEBUG_MIIO("[SerialMIIO]no ack result callback");
  } else {

    auto callback = _ackResultCallback;
    _ackResultCallback = NULL;
    callback(result);
  }
}

void SerialMIIO::_handleXiaomiSetup(bool result) {
  DEBUG_MIIO(
      "[SerialMIIO]handle xiaomi setup, %d, result %s",
      _setupStatus,
      result ? "success" : "failed");

  String sendCmd;
  sendCmd.reserve(CMD_BUF_SIZE);

  switch (_setupStatus) {
  case SETUP_ECHO:
    if (result) {
      _setupStatus = SETUP_MODEL;
    } else {
      sendCmd += "echo off";

      sendStrWaitAck(
          sendCmd,
          std::bind(
              &SerialMIIO::_handleXiaomiSetup, this, std::placeholders::_1));
    }
    break;
  case SETUP_MODEL:
    if (result) {
      _setupStatus = SETUP_BLE_PID;
    } else {
      sendCmd += "model ";
      sendCmd += _model;
      sendStrWaitAck(
          sendCmd,
          std::bind(
              &SerialMIIO::_handleXiaomiSetup, this, std::placeholders::_1));
    }
    break;
  case SETUP_BLE_PID:
    if (result) {
      _setupStatus = SETUP_MCU_VERSION;

    } else {
      sendCmd += "ble_config set ";
      sendCmd += _blePid;
      sendCmd += " ";
      sendCmd += _mcuVersion;
      sendStrWaitAck(
          sendCmd,
          std::bind(
              &SerialMIIO::_handleXiaomiSetup, this, std::placeholders::_1));
    }
    break;
  case SETUP_MCU_VERSION:
    if (result) {
      _setupStatus = SETUP_OK;
    } else {
      sendCmd += "mcu_version ";
      sendCmd += _mcuVersion;
      sendStrWaitAck(
          sendCmd,
          std::bind(
              &SerialMIIO::_handleXiaomiSetup, this, std::placeholders::_1));
    }
    break;
  }
}

void SerialMIIO::_handleGetDown(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]handle get down: %s", cmd.c_str());
  _needGetDown = true;

  char method[CMD_BUF_SIZE] = {0};
  size_t methodLen = sizeof(method);
  int ret =
      uart_comamnd_decoder(_cmd.c_str(), cmd.length(), method, &methodLen);
  if (MIIO_OK != ret) { /* judge if string decoded correctly */
    DEBUG_MIIO("[SerialMIIO]get method failed");
    return;
  }

  if (methodLen > 0 && method != NULL) { /* start to find if method contained */
    auto callback = callbackFindByMethod(method);
    if (NULL == callback) {
      if (strcmp(ERROR_STRING, method) && strcmp(OK_STRING, method)) {
        sendErrorCode(ERROR_MESSAGE_UNCMD, ERROR_CODE_UNCMD);
        DEBUG_MIIO("[SerialMIIO]undefined method: %s", method);
      }
    } else {
      DEBUG_MIIO("[SerialMIIO]found method: %s", method);
      callback(_cmd.c_str(), cmd.length());
    }
  } else {
    DEBUG_MIIO("[SerialMIIO]unknown method: %s", method);
  }
}

void SerialMIIO::_handleAck(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]handle ack: %s", cmd.c_str());

  bool isOk = cmd.startsWith(OK_STRING);
  if (!isOk) {
    DEBUG_MIIO("[SerialMIIO]send string wait ack failed");
  }

  _executeackResultCallbacks(isOk);
}
