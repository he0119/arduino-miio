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
#define ERROR_STRING "error"
#define OK_STRING "ok"
#define SET_PRO_STRING "set_properties"
#define GET_PRO_STRING "get_properties"
#define ACTION_STRING "action"
#define NET_CHANGE_STRING "MIIO_net_change"
#define NONE_STRING "none"
#define MCU_VERSION_REQ_STRING "MIIO_mcu_version_req"

/* ==================== buffer size ==================== */
/* buffer used to receive command string from wifi module */
#define CMD_STR_MAX_LEN (800) // max 800 byte
/* max method name register with miio_cmd_method_register() function */
#define CMD_METHOD_LEN_MAX (64)
/* buffer used to response command execute result */
#define RESULT_BUF_SIZE CMD_STR_MAX_LEN

#define ACK_BUF_SIZE CMD_STR_MAX_LEN
#define DATA_STRING_MAX_LENGTH (800)

/* ==================== operation define ==================== */
#define ID_MAX_LEN 4
#define VALUE_MAX_LEN 16

#define ERROR_MESSAGE_UNCMD "undefined command"
#define ERROR_MESSAGE_UNPARAMS "error command params"
#define ERROR_CODE_UNCMD -9999
#define ERROR_CODE_UNPARAMS -9998

#define USER_POLL_INTERVAL_MS (200)
#define USER_UART_TIMEOUT_MS (200)
#define USER_RECEIVE_RETRY (25)

#endif
