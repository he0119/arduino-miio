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
#ifndef _MIIO_H_
#define _MIIO_H_

#include "Arduino.h"
#include <map>

 /* ==================== debug define ==================== */
#ifndef NODEBUG_MIIO
#ifdef DEBUG_ESP_PORT
#define DEBUG_MIIO(...)               \
    {                                       \
        DEBUG_ESP_PORT.printf(__VA_ARGS__); \
        DEBUG_ESP_PORT.flush();             \
    }
#endif
#endif

#ifndef DEBUG_MIIO
#define DEBUG_MIIO(...)
#ifndef NODEBUG_MIIO
#define NODEBUG_MIIO
#endif
#endif

 /* ==================== error define ==================== */
#define MIIO_OK					(0)		/* There is no error		*/
#define MIIO_ERROR				(-1)	/* A generic error happens	*/
#define MIIO_ERROR_TIMEOUT		(-2)	/* Timed out				*/
#define MIIO_ERROR_FULL			(-3)	/* The resource is full		*/
#define MIIO_ERROR_EMPTY		(-4)	/* The resource is empty	*/
#define MIIO_ERROR_NOMEM		(-5)	/* No memory				*/
#define MIIO_ERROR_NOSYS		(-6)	/* No system				*/
#define MIIO_ERROR_BUSY			(-7)	/* Busy						*/
#define MIIO_ERROR_TRYOUT		(-8)	/* Try enough times			*/
#define MIIO_ERROR_NOTFOUND		(-9)
#define MIIO_ERROR_PARAM		(-10)
#define MIIO_ERROR_SIZE			(-11)
#define MIIO_ERROR_NOTREADY		(-12)

enum uart_error_t {
  UART_OK = 0,
  UART_DESTROY_ERROR = -1,
  UART_OPEN_ERROR = -2,
  UART_SET_ARRT_ERROR = -3,
  UART_SEND_ERROR = -4,
  UART_RECV_ACK_ERROR = -5,
  UART_RECV_ERROR = -6,
};

/* ==================== common string constants ==================== */
#define SPACE_CHAR					' '
#define SPACE_STRING				" "
#define END_CHAR					'\r'
#define ERROR_STRING				"error"
#define OK_STRING				    "ok"
#define SET_PRO_STRING				"set_properties"
#define GET_PRO_STRING				"get_properties"
#define NET_CHANGE_STRING			"MIIO_net_change"
#define NONE_STRING					"none"

/* ==================== buffer size ==================== */
/* buffer used to receive command string from wifi module */
#define CMD_STR_MAX_LEN             (800) //max 800 byte
/* max method name register with miio_cmd_method_register() function */
#define CMD_METHOD_LEN_MAX          (64)
/* buffer used to response command execute result */
#define RESULT_BUF_SIZE             CMD_STR_MAX_LEN

#define ACK_BUF_SIZE                CMD_STR_MAX_LEN
#define DATA_STRING_MAX_LENGTH      (800)

/* ==================== function define ==================== */
typedef std::function<int(char* cmd, size_t length)> MethodCallback;


/* ==================== property define ==================== */
enum property_format_t
{
  PROPERTY_FORMAT_UNDEFINED = 0,
  PROPERTY_FORMAT_BOOLEAN = 1,
  PROPERTY_FORMAT_STRING = 2,
  PROPERTY_FORMAT_NUMBER = 3,
};

struct data_boolean_t
{
  bool value;
};

struct data_string_t
{
  char value[DATA_STRING_MAX_LENGTH + 1];
  uint32_t length;
};

struct data_number_value_t
{
  long integerValue;
  float floatValue;
};

enum data_number_type_t
{
  DATA_NUMBER_INTEGER = 0,
  DATA_NUMBER_FLOAT = 1,
};

struct data_number_t
{
  data_number_type_t  type;
  data_number_value_t value;
};

union property_data_t
{
  data_boolean_t     boolean;
  data_string_t      string;
  data_number_t      number;
};
struct property_value_t
{
  property_format_t   format;
  property_data_t     data;
};

enum property_operation_type
{
  PROPERTY_OPERATION_GET = 0,
  PROPERTY_OPERATION_SET = 1,
};

struct property_operation_t
{
  uint32_t            siid;
  uint32_t            piid;
  int                 code;
  property_value_t* value;
};

class MIIO
{
public:
  MIIO(Stream& serial);

  void begin(const char* model, const char* blePid, const char* mcuVersion);

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

  int onCommand(String method, MethodCallback callback);

  MethodCallback callbackFindByMethod(const char* method);

  size_t sendStr(const char* str);

  size_t sendStr(String str);

  size_t sendStrWaitAck(const char* str);

  size_t sendStrWaitAck(String str);

  size_t recvStr(char* buffer, size_t length);

  int sendPropertyChanged(uint32_t siid, uint32_t piid, property_value_t* newValue);

  int executePropertyChanged(property_operation_t& opt);

  int uartComamndDecoder(char* pbuf, size_t buf_sz, char* method, size_t* methodLen);


private:
  Stream* _serial;
  unsigned long _pollIntervalMs = 200;
  unsigned long _lastPoll = 0;
  unsigned int _receiveRetry = 25;

  char _pbuf[CMD_STR_MAX_LEN] = { 0 };
  char _method[CMD_METHOD_LEN_MAX] = { 0 };
  std::map<String, MethodCallback> _commands;
};

#endif
