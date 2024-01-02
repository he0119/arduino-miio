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

SerialMIIO::SerialMIIO(Stream &stream) {
  _stream = &stream;
  commonInit();
}

SerialMIIO::SerialMIIO(HardwareSerial &serial) {
  serial.begin(115200);
  _stream = &serial;
  commonInit();
}

#ifdef USE_SW_SERIAL
SerialMIIO::SerialMIIO(SoftwareSerial &serial) {
  serial.begin(115200);
  _stream = &serial;
  commonInit();
}
#endif

void SerialMIIO::commonInit() {
  _sendBuffer.reserve(CMD_BUF_SIZE);
  _recvBuffer.reserve(CMD_BUF_SIZE);

  _state = STATE_SETUP;

  // 注册默认的回调函数
  // 用于处理 get_properties/set_properties/action/none/mcu_version_req
  // 可以被用户覆盖
  onMethod(GET_PRO_STRING, [this](const char *cmd, uint32_t length) {
    _defaultGetPropertiesCallback(cmd, length);
  });
  onMethod(SET_PRO_STRING, [this](const char *cmd, uint32_t length) {
    _defaultSetPropertyCallback(cmd, length);
  });
  onMethod(ACTION_STRING, [this](const char *cmd, uint32_t length) {
    _defaultinvokeActionCallback(cmd, length);
  });
  onMethod(NONE_STRING, [this](const char *cmd, uint32_t length) {
    _defaultInvokeNoneCallback(cmd, length);
  });
  onMethod(MCU_VERSION_REQ_STRING, [this](const char *cmd, uint32_t length) {
    _defaultMCUVersionCallback(cmd, length);
  });
}

void SerialMIIO::begin(
    const char *model, const char *blePid, const char *mcuVersion) {
  begin(String(model), String(blePid), String(mcuVersion));
}

void SerialMIIO::begin(String model, String blePid, String mcuVersion) {
  _model = model;
  _blePid = blePid;
  _mcuVersion = mcuVersion;
}

void SerialMIIO::setTimeout(uint32_t timeout) {
  _timeoutMs = timeout;
}

void SerialMIIO::setPollInterval(uint32_t interval) {
  _pollIntervalMs = interval;
}

void SerialMIIO::setMaxRetry(uint32_t count) {
  _maxRetry = count;
}

void SerialMIIO::setLogLevel(uint8_t level) {
  if (level > 5 || level < 0) {
    return;
  }
  _sendBuffer = "set_log_level " + String(level);
  sendWaitAck();
}

int32_t SerialMIIO::send(ReceiveCallback callback) {
  _recvCallback = callback;

  if (_sendBuffer.length() == 0) {
    return UART_OK;
  }
  uint32_t length = _sendBuffer.length();

  uint32_t nSend = _stream->print(_sendBuffer);
  // 如果字符串没有以 \r 结尾，则补上
  if (!_sendBuffer.endsWith("\r")) {
    nSend += _stream->print("\r");
    length += 1;
  }
  _startMillis = millis();

  DEBUG_MIIO("[SerialMIIO]send string: ");
  DEBUG_MIIO(_sendBuffer.c_str());

  if (nSend < _sendBuffer.length()) {
    DEBUG_MIIO("[SerialMIIO]send string failed");
    return UART_SEND_ERROR;
  }

  return nSend;
}

int32_t SerialMIIO::sendWaitAck() {
  if (_sendBuffer.length() == 0) {
    return UART_OK;
  }

  int32_t nSend = send([this](String &cmd) { _handleAck(cmd); });

  return nSend;
}

int32_t SerialMIIO::sendWaitAck(AckCallback callback) {
  _ackCallback = callback;
  return sendWaitAck();
}

int32_t SerialMIIO::sendResponse() {
  int8_t ret = MIIO_OK;

  if (_sendBuffer.length() == 0) {
    DEBUG_MIIO("[SerialMIIO]response is empty");
    return MIIO_ERROR;
  }

  int32_t nSend = sendWaitAck();

  if (nSend < 0) {
    DEBUG_MIIO("[SerialMIIO]stream send result failed");
    return MIIO_ERROR;
  }

  if (nSend < _sendBuffer.length()) {
    DEBUG_MIIO("[SerialMIIO]stream send result incomplete");
    ret = MIIO_ERROR;
  }

  _state = STATE_WAIT_ACK;
  return ret;
}

int32_t SerialMIIO::sendErrorCode(const String &msg, int32_t errcode) {
  _sendBuffer = "error ";
  _sendBuffer += "\"";
  _sendBuffer += msg;
  _sendBuffer += "\" ";
  _sendBuffer += String(errcode);

  return sendResponse();
}

int32_t SerialMIIO::sendErrorCode(const char *msg, int32_t errcode) {
  return sendErrorCode(String(msg), errcode);
}

int32_t SerialMIIO::sendPropertyChanged(
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

  _sendBuffer = String(out);
  ret = sendResponse();

  if (ret != MIIO_OK) {
    DEBUG_MIIO("[SerialMIIO]send property changed failed");
  } else {
    DEBUG_MIIO("[SerialMIIO]send property changed success");
  }

  miio_property_operation_delete(opt);

  return ret;
}

int32_t SerialMIIO::sendEventOccurred(event_operation_t *event) {
  DEBUG_MIIO("[SerialMIIO]event occurred");

  int32_t ret = MIIO_OK;

  if (NULL == event) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  char out[CMD_BUF_SIZE] = {0};
  memset(out, 0, CMD_BUF_SIZE);

  miio_event_operation_encode(event, out, CMD_BUF_SIZE);

  _sendBuffer = String(out);
  ret = sendResponse();

  if (MIIO_OK == ret) {
    DEBUG_MIIO("[SerialMIIO]event send success");
  } else {
    DEBUG_MIIO("[SerialMIIO]event send failed");
  }

  return ret;
}

int32_t SerialMIIO::executePropertyOperation(
    const char *cmd, uint32_t length, property_operation_type type) {
  int32_t ret = MIIO_OK;

  char *cmdBuf = (char *)calloc(1, length);
  if (NULL == cmdBuf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmdBuf, cmd, length - 1);
  cmdBuf[length - 1] = '\0'; /* remove '\r' at the end */

  uint16_t paramsPairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmdBuf, " "); /* pass string "down" */
  temp = strtok(NULL, " "); /* pass string "get_properties"/"set_properties" */

  while (true) {
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

    for (uint16_t i = 0; i < paramsPairs; ++i) {
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

    _sendBuffer = String(result);
    sendResponse();
  } while (false);

  if (NULL != cmdBuf) {
    free(cmdBuf);
  }

  return ret;
}

int32_t SerialMIIO::executeActionInvocation(const char *cmd, uint32_t length) {
  int32_t ret = MIIO_OK;

  char *cmdBuf = (char *)calloc(1, length);
  if (NULL == cmdBuf) {
    DEBUG_MIIO("[SerialMIIO]command buf malloc failed");
    return MIIO_ERROR;
  }

  memcpy(cmdBuf, cmd, length - 1);
  cmdBuf[length - 1] = '\0'; /* remove '\r' at the end */
  uint16_t paramsPairs = 0;

  /* don't need to check temp, cause it checked before enter into this func */
  char *temp = strtok(cmdBuf, " "); /* pass string "down" */
  temp = strtok(NULL, " ");         /* pass string "action" */

  while (true) {
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

    for (uint16_t i = 0; i < paramsPairs; ++i) {
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

    _sendBuffer = String(result);
    sendResponse();
  } while (false);

  if (NULL != cmdBuf) {
    free(cmdBuf);
  }

  return ret;
}

void SerialMIIO::onMethod(String method, MethodCallback callback) {
  if (method.length() == 0 || callback == NULL) {
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
  _state = STATE_WAIT_REPLY;
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
  _state = STATE_WAIT_REPLY;
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
  _state = STATE_WAIT_REPLY;
}

void SerialMIIO::_defaultGetPropertiesCallback(
    const char *cmd, uint32_t length) {
  DEBUG_MIIO("[SerialMIIO]down get_properties default callback");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_GET);
}

void SerialMIIO::_defaultSetPropertyCallback(const char *cmd, uint32_t length) {
  DEBUG_MIIO("[SerialMIIO]down set_properties default callback");

  executePropertyOperation(cmd, length, PROPERTY_OPERATION_SET);
}

void SerialMIIO::_defaultinvokeActionCallback(
    const char *cmd, uint32_t length) {
  DEBUG_MIIO("[SerialMIIO]down action default callback");

  executeActionInvocation(cmd, length);
}

void SerialMIIO::_defaultInvokeNoneCallback(const char *cmd, uint32_t length) {
  DEBUG_MIIO("[SerialMIIO]down none default callback");
}

void SerialMIIO::_defaultMCUVersionCallback(const char *cmd, uint32_t length) {
  DEBUG_MIIO("[SerialMIIO]down mcu_version_req default callback");

  _sendBuffer = "mcu_version ";
  _sendBuffer += _mcuVersion;
  sendResponse();
}

void SerialMIIO::handle() {
  switch (_state) {
  case STATE_SETUP:
    // 正在初始化
    if (_setupStatus == SETUP_OK) {
      _state = STATE_IDLE;
      break;
    }
    _handleSetup(false);
    _recvStr();
    break;
  case STATE_IDLE:
    // 空闲状态
    if (millis() - _lastPollMillis < _pollIntervalMs && _lastPollMillis != 0) {
      // 没有达到轮询时间
      // 状态不改变
      return;
    }
    _sendGetDown();
    _state = STATE_WAIT_GET_DOWN;
    break;
  case STATE_WAIT_GET_DOWN:
    // 等待 get_down 结果
    _recvStr(); // To: WAIT_REPLY or WAIT_ACK, IDLE if timeout
    break;
  case STATE_WAIT_REPLY:
    // 等待回复
    if (_recvStr()) {
      _state = STATE_WAIT_ACK;
    }
    break;
  case STATE_WAIT_ACK:
    // 等待回复
    if (_recvStr()) {
      _state = STATE_IDLE;
    }
    break;
  }
}

void SerialMIIO::_sendGetDown() {
  _lastPollMillis = millis();
  _sendBuffer = String(GET_DOWN_STRING);

  send([this](String &cmd) { _handleGetDown(cmd); });
}

bool SerialMIIO::_recvStr() {
  // 返回 true 时表示处理完成

  // 超时 增加重试次数
  if (_stream->available() <= 0 && millis() - _startMillis > _timeoutMs) {
    DEBUG_MIIO("[SerialMIIO]receive timeout");
    _retry++;
    _startMillis = millis();
    _clearRecvBuffer();
    // 超时重传 保留回调函数
    DEBUG_MIIO("[SerialMIIO]resend");
    send(_recvCallback);
    return false;
  }

  while (_stream->available() > 0) {
    _recvBuffer += (char)_stream->read();
    _startMillis = millis();

    if (_recvBuffer.length() > CMD_BUF_SIZE) {
      DEBUG_MIIO(
          "[SerialMIIO]receive cmd too long %d bytes", _recvBuffer.length());
      _clearRecvBuffer();
      continue;
    }

    if (_recvBuffer.endsWith(END_STRING)) {
      DEBUG_MIIO("[SerialMIIO]receive cmd end");
      DEBUG_MIIO("[SerialMIIO]receive buffer: ");
      DEBUG_MIIO(_recvBuffer.c_str());
      _executeReceiveCallback(_recvBuffer);
      return true;
    }
  }

  if (_retry > _maxRetry) {
    DEBUG_MIIO("[SerialMIIO]receive retry too many times");
    _executeReceiveCallback(_recvBuffer);
    _state = STATE_IDLE;
    return true;
  }

  return false;
}

void SerialMIIO::_clearRecvBuffer() {
  _recvBuffer = String();
  _retry = 0;
}

void SerialMIIO::_executeReceiveCallback(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]execute receive callback");

  if (NULL == _recvCallback) {
    DEBUG_MIIO("[SerialMIIO]no receive callback");
  } else {
    auto callback = _recvCallback;
    _recvCallback = NULL;
    callback(cmd);
  }

  // 无论什么时候，执行完回调后，都清空缓存
  _clearRecvBuffer();
}

void SerialMIIO::_executeAckCallback(bool result) {
  DEBUG_MIIO("[SerialMIIO]execute ack result callback");

  if (NULL == _ackCallback) {
    DEBUG_MIIO("[SerialMIIO]no ack result callback");
  } else {
    auto callback = _ackCallback;
    _ackCallback = NULL;
    callback(result);
  }

  if (_state == STATE_WAIT_ACK) {
    _state = STATE_IDLE;
  }
}

void SerialMIIO::_handleSetup(bool result) {
  switch (_setupStatus) {
  case SETUP_ECHO:
    if (result) {
      _setupStatus = SETUP_MODEL;
    } else {
      _sendBuffer = "echo off";

      sendWaitAck([this](bool result) { _handleSetup(result); });
    }
    break;
  case SETUP_MODEL:
    if (result) {
      _setupStatus = SETUP_BLE_PID;
    } else {
      _sendBuffer = "model ";
      _sendBuffer += _model;
      sendWaitAck([this](bool result) { _handleSetup(result); });
    }
    break;
  case SETUP_BLE_PID:
    if (result) {
      _setupStatus = SETUP_MCU_VERSION;

    } else {
      _sendBuffer = "ble_config set ";
      _sendBuffer += _blePid;
      _sendBuffer += " ";
      _sendBuffer += _mcuVersion;
      sendWaitAck([this](bool result) { _handleSetup(result); });
    }
    break;
  case SETUP_MCU_VERSION:
    if (result) {
      _setupStatus = SETUP_OK;
    } else {
      _sendBuffer = "mcu_version ";
      _sendBuffer += _mcuVersion;
      sendWaitAck([this](bool result) { _handleSetup(result); });
    }
    break;
  case SETUP_OK:
    break;
  }
}

void SerialMIIO::_handleGetDown(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]handle get down: ");
  DEBUG_MIIO(cmd.c_str());

  char method[CMD_BUF_SIZE] = {0};
  uint32_t methodLen = sizeof(method);
  int ret = uart_comamnd_decoder(
      _recvBuffer.c_str(), cmd.length(), method, &methodLen);
  if (MIIO_OK != ret) { /* judge if string decoded correctly */
    DEBUG_MIIO("[SerialMIIO]get method failed");
    return;
  }

  if (methodLen > 0 && method != NULL) { /* start to find if method contained */
    auto callback = callbackFindByMethod(method);
    if (NULL == callback) {
      if (strcmp(ERROR_STRING, method) && strcmp(OK_STRING, method)) {
        sendErrorCode(ERROR_MESSAGE_UNCMD, ERROR_CODE_UNCMD);
        DEBUG_MIIO("[SerialMIIO]undefined method: ");
        DEBUG_MIIO(method);
      }
    } else {
      DEBUG_MIIO("[SerialMIIO]found method: ");
      DEBUG_MIIO(method);
      callback(_recvBuffer.c_str(), cmd.length());
    }
  } else {
    DEBUG_MIIO("[SerialMIIO]unknown method: ");
    DEBUG_MIIO(method);
  }
}

void SerialMIIO::_handleAck(String &cmd) {
  DEBUG_MIIO("[SerialMIIO]handle ack: ");
  DEBUG_MIIO(cmd.c_str());

  bool isOk = cmd.startsWith(OK_STRING);
  if (!isOk) {
    DEBUG_MIIO("[SerialMIIO]send string wait ack failed");
  }

  _executeAckCallback(isOk);
}
