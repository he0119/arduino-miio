#ifndef _SerialMIIO_H_
#define _SerialMIIO_H_

#if (                                                                          \
    defined(__AVR__) || ((defined(ARDUINO_UNOR4_WIFI) || defined(ESP8266)) &&  \
                         !defined(NO_SW_SERIAL)))
#define USE_SW_SERIAL
#endif

#include <Arduino.h>

#ifdef USE_SW_SERIAL
#include <SoftwareSerial.h>
#endif

#include <map>
#include <vector>

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

class SerialMIIO {
  using MethodCallback = std::function<void(const char *cmd, uint32_t length)>;
  using PropertyCallback = std::function<void(property_operation_t *o)>;
  using ActionInvokeCallback = std::function<void(action_operation_t *o)>;
  using ReceiveCallback = std::function<void(String &cmd)>;
  using AckResultCallback = std::function<void(int result)>;

  enum SetupStatus {
    SETUP_ECHO,
    SETUP_MODEL,
    SETUP_BLE_PID,
    SETUP_MCU_VERSION,
    SETUP_OK,
  };

public:
  SerialMIIO(Stream &stream);
  SerialMIIO(HardwareSerial &serial);
#ifdef USE_SW_SERIAL
  SerialMIIO(SoftwareSerial &serial);
#endif

  /**
   * @brief 小米模组基本设置
   *
   * 完成小米模组的基本设置，每次上电后都需要调用此函数。
   *
   * @param model 产品型号，如 xiaomi.prod.v2（不能超过 23 个字符）
   * @param blePid 模组 PID（小于 65536）
   * @param mcuVersion MCU 固件版本（必须是 4 位数字）
   */
  void begin(const char *model, const char *blePid, const char *mcuVersion);
  void begin(String model, String blePid, String mcuVersion);

  void common_init(void);

  void handle();

  /**
   * @brief 设置串口超时时间
   * @note 默认为 200 毫秒
   * @param timeout 超时时间（毫秒）
   */
  void setTimeout(uint32_t timeout);

  /**
   * @brief 设置轮询间隔时间。
   *
   * @note 时间范围要求 100~200ms，推荐的循环周期为 200ms，默认为 200ms。
   * https://iot.mi.com/v2/new/doc/embedded-dev/module-dev/function-dev/mcu-dev#命令规范
   * @param interval 轮询间隔时间，单位为毫秒。
   */
  void setPollInterval(uint32_t interval);

  /**
   * @brief 设置最大接收重试次数
   *
   * @note 默认为 25 次
   * @param count 最大重试次数
   */
  void setMaxRetry(uint32_t count);

  /**
   * @brief 设置日志等级
   *
   * @note 0 对应 debug, 5 对应不输出日志
   * @param level 日志等级
   */
  void setLogLevel(uint8_t level);

  void onMethod(String method, MethodCallback callback);
  void
  onActionInvoke(uint32_t siid, uint32_t aiid, ActionInvokeCallback callback);
  void onPropertyGet(uint32_t siid, uint32_t piid, PropertyCallback callback);
  void onPropertySet(uint32_t siid, uint32_t piid, PropertyCallback callback);

  MethodCallback callbackFindByMethod(const char *method);
  ActionInvokeCallback callbackFindByAction(uint32_t siid, uint32_t aiid);
  PropertyCallback callbackFindByPropertyGet(uint32_t siid, uint32_t piid);
  PropertyCallback callbackFindByPropertySet(uint32_t siid, uint32_t piid);

  int32_t sendStr(const String &str, ReceiveCallback callback);
  int32_t sendStr(const char *str, ReceiveCallback callback);
  int32_t sendStrWaitAck(const String &str);
  int32_t sendStrWaitAck(const char *str);
  int32_t sendStrWaitAck(const String &str, AckResultCallback callback);
  int32_t sendStrWaitAck(const char *str, AckResultCallback callback);

  /**
   * @brief 发送回复
   * @param response 回复内容
   * @return 发送状态，0 为成功，其他为失败
   */
  int32_t sendResponse(const char *response);
  int32_t sendResponse(const String &response);

  int32_t sendErrorCode(const char *msg, int32_t errcode);
  int32_t sendErrorCode(const String &msg, int32_t errcode);

  int32_t
  sendPropertyChanged(uint32_t siid, uint32_t piid, property_value_t *newValue);
  int32_t sendEventOccurred(event_operation_t *event);

  int32_t executePropertyOperation(
      const char *cmd, uint32_t length, property_operation_type type);
  int32_t executeActionInvocation(const char *cmd, uint32_t length);

private:
  Stream *_stream;

  String _model;
  String _blePid;
  String _mcuVersion;

  SetupStatus _setupStatus = SETUP_ECHO;
  // 是否发送了 get_down
  bool _getDownSent = false;

  uint32_t _lastPollMillis = 0;
  uint32_t _pollIntervalMs = USER_POLL_INTERVAL_MS;

  uint32_t _startMillis = 0;
  uint32_t _timeoutMs = USER_UART_TIMEOUT_MS;

  String _cmd;
  uint16_t _retry = 0;
  uint16_t _maxRetry = USER_RECEIVE_RETRY;
  void _clearReceiveBuffer();

  void _sendGetDown();
  void _recvStr();

  void _executeReceiveCallback(String &cmd);
  void _executeackResultCallback(bool result);

  AckResultCallback _ackResultCallback;
  ReceiveCallback _receiveCallback;

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
  void _defaultGetPropertiesCallback(const char *cmd, uint32_t length);
  void _defaultSetPropertyCallback(const char *cmd, uint32_t length);
  void _defaultinvokeActionCallback(const char *cmd, uint32_t length);
  void _defaultinvokeNoneCallback(const char *cmd, uint32_t length);
  void _defaultMCUVersionCallback(const char *cmd, uint32_t length);

  void _handleXiaomiSetup(bool result);
  void _handleGetDown(String &cmd);
  void _handleAck(String &cmd);
};

#endif
