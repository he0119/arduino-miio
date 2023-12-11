#ifndef _MIIO_H_
#define _MIIO_H_

#include <map>

#include "Arduino.h"

extern "C" {
#include "miio/device/codec/action_operation_decoder.h"
#include "miio/device/codec/action_operation_encoder.h"
#include "miio/device/codec/event_operation_encoder.h"
#include "miio/device/codec/property_changed_encoder.h"
#include "miio/device/codec/property_operation_decoder.h"
#include "miio/device/codec/property_operation_encoder.h"
#include "miio/device/codec/uart_command_decoder.h"
#include "miio/device/typedef/event_operation.h"
#include "miio/device/typedef/operation_code.h"
#include "miio/device/typedef/property_operation.h"
#include "miio/device/typedef/property_value.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"
}

/* ==================== debug define ==================== */
#ifndef NODEBUG_MIIO
#ifdef DEBUG_ESP_PORT
#define DEBUG_MIIO(...)                                                        \
  {                                                                            \
    DEBUG_ESP_PORT.printf(__VA_ARGS__);                                        \
    DEBUG_ESP_PORT.println();                                                  \
    DEBUG_ESP_PORT.flush();                                                    \
  }
#endif
#endif

#ifndef DEBUG_MIIO
#define DEBUG_MIIO(...)
#ifndef NODEBUG_MIIO
#define NODEBUG_MIIO
#endif
#endif

/* ==================== callback function define ==================== */
typedef std::function<int(char *cmd, size_t length)> MethodCallback;
typedef std::function<void(property_operation_t *o)> PropertyCallback;
typedef std::function<void(action_operation_t *o)> ActionInvokeCallback;

class MIIO {
public:
  MIIO(Stream &serial);

  void begin(const char *model, const char *blePid, const char *mcuVersion);

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

  int onMethod(String method, MethodCallback callback);

  int onActionInvoke(
      uint32_t siid, uint32_t aiid, ActionInvokeCallback callback);

  int onPropertyGet(uint32_t siid, uint32_t piid, PropertyCallback callback);

  int onPropertySet(uint32_t siid, uint32_t piid, PropertyCallback callback);

  MethodCallback callbackFindByMethod(const char *method);

  ActionInvokeCallback callbackFindByAction(uint32_t siid, uint32_t aiid);

  PropertyCallback callbackFindByPropertyGet(uint32_t siid, uint32_t piid);

  PropertyCallback callbackFindByPropertySet(uint32_t siid, uint32_t piid);

  size_t sendStr(const char *str);

  size_t sendStr(String str);

  size_t sendStrWaitAck(const char *str);

  size_t sendStrWaitAck(String str);

  size_t recvStr(char *buffer, size_t length);

  int sendPropertyChanged(
      uint32_t siid, uint32_t piid, property_value_t *newValue);

  int executePropertyChanged(property_operation_t *opt);

  int sendEventOccurred(event_operation_t *event);

  int eventSend(char out[], size_t len);

  /**
   * ------------------------------------------------
   * request : down get_properties 1 1 2 1
   * ------------------------------------------------
   * response : result 1 1 0 "hello" 2 1 0 true
   * ------------------------------------------------
   *
   *
   * ------------------------------------------------
   * request : down set_properties 1 1 "hello" 2 1 true
   * ------------------------------------------------
   * response : result 1 1 0 2 1 0
   * ------------------------------------------------
   */
  int executePropertyOperation(
      const char *pbuf, int buf_sz, property_operation_type type);

  /**
   * ------------------------------------------------
   * request : down action 4 1 1 false
   * ------------------------------------------------
   * response : result 4 1 0 1 false
   * ------------------------------------------------
   */
  int executeActionInvocation(const char *pbuf, int buf_sz);

  int sendResponse(const char *pbuf);

  int sendErrorCode(const char *pbuf, int errcode);

private:
  Stream *_serial;

  unsigned long _lastPoll = 0;
  unsigned long _pollIntervalMs = USER_POLL_INTERVAL_MS;
  unsigned int _receiveRetry = USER_RECEIVE_RETRY;

  char _pbuf[CMD_STR_MAX_LEN] = {0};
  char _method[CMD_METHOD_LEN_MAX] = {0};

  std::map<String, MethodCallback> _methodCallbacks;
  std::map<std::pair<uint32_t, uint32_t>, PropertyCallback>
      _propertyGetCallbacks;
  std::map<std::pair<uint32_t, uint32_t>, PropertyCallback>
      _propertySetCallbacks;
  std::map<std::pair<uint32_t, uint32_t>, ActionInvokeCallback>
      _actionInvokeCallbacks;

  void _onPropertyGet(property_operation_t *o);
  void _onPropertySet(property_operation_t *o);
  void _onActionInvoke(action_operation_t *o);
  int _defaultGetPropertiesCallback(char *cmd, size_t length);
  int _defaultSetPropertyCallback(char *cmd, size_t length);
  int _defaultinvokeActionCallback(char *cmd, size_t length);
};

#endif
