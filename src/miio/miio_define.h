/**
 * @author  MIoT
 * @date    2019
 * @par     Copyright (c):
 *
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
#ifndef __MIIO_DEFINE_H__
#define __MIIO_DEFINE_H__

#include "Arduino.h"

/* ==================== error define ==================== */
#define MIIO_OK (0)             /* There is no error		*/
#define MIIO_ERROR (-1)         /* A generic error happens	*/
#define MIIO_ERROR_TIMEOUT (-2) /* Timed out				*/
#define MIIO_ERROR_FULL (-3)    /* The resource is full		*/
#define MIIO_ERROR_EMPTY (-4)   /* The resource is empty	*/
#define MIIO_ERROR_NOMEM (-5)   /* No memory				*/
#define MIIO_ERROR_NOSYS (-6)   /* No system				*/
#define MIIO_ERROR_BUSY (-7)    /* Busy    */
#define MIIO_ERROR_TRYOUT (-8)  /* Try enough times			*/
#define MIIO_ERROR_NOTFOUND (-9)
#define MIIO_ERROR_PARAM (-10)
#define MIIO_ERROR_SIZE (-11)
#define MIIO_ERROR_NOTREADY (-12)

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
#define SPACE_CHAR ' '
#define SPACE_STRING " "
#define END_CHAR '\r'
#define END_STRING "\r"
#define ERROR_STRING "error"
#define OK_STRING "ok"
#define GET_DOWN_STRING "get_down"
#define SET_PRO_STRING "set_properties"
#define GET_PRO_STRING "get_properties"
#define ACTION_STRING "action"
#define NET_CHANGE_STRING "MIIO_net_change"
#define NONE_STRING "none"
#define MCU_VERSION_REQ_STRING "MIIO_mcu_version_req"

/* ==================== max define ==================== */
// model 字符串由公司名、产品类别名和产品版本构成，总长度不能超过 23 个字符。
#define MODEL_NAME_MAX_LEN (23)
// 双模模组的蓝牙 PID（使用十进制；蓝牙配网使用，PID 小于 65536）
#define BLE_PID_MAX_VALUE (65536)
// MCU 固件版本必须是 4 位数字
#define MCU_VERSION_LEN (4)
// result/properties_changed/event_occured
// 整个指令最多 512 字节。
#define CMD_MAX_LEN (512)
// 指令后面最多使用 64个参数
#define CMD_PARAM_MAX_LEN (64)
/* 指令名称的最大长度 */
#define CMD_METHOD_MAX_LEN (64)

/* ==================== buffer size ==================== */
// 指令标准是不超过 512 字节，为了防止溢出，这里设置为 800 字节
#define CMD_BUF_SIZE (800)
#define ACK_BUF_SIZE CMD_BUF_SIZE

#define ERROR_MESSAGE_UNCMD "undefined command"
#define ERROR_MESSAGE_UNPARAMS "error command params"
#define ERROR_MESSAGE_RESULT_TOO_LONG "result too long"
#define ERROR_CODE_UNCMD -9999
#define ERROR_CODE_UNPARAMS -9998
#define ERROR_CODE_RESULT_TOO_LONG -9997

#define USER_POLL_INTERVAL_MS (200)
#define USER_UART_TIMEOUT_MS (200)
#define USER_RECEIVE_RETRY (25)

#endif
