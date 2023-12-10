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

#define MIIO_BASE64_SIZE(len)		(((len)+2)/3*4+1)

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

#define MIIO_ITERATOR_CONTINUE 			MIIO_OK
#define MIIO_ITERATOR_ABORT 			MIIO_ERROR

/* ==================== system error code ==================== */
/* =================== -32767 < x < -30000 =================== */
#define MIIO_OT_ERR_CODE_MIN				(-32767)
#define MIIO_OT_ERR_CODE_MAX				(-30000)
#define MIIO_OT_ERR_CODE_R(e)				(MIIO_OT_ERR_CODE_MIN+abs(e))
#define MIIO_OT_ERR_CODE_L(e)				(MIIO_OT_ERR_CODE_MAX-abs(e))
#define MIIO_OT_ERR_CODE_FILTER(e)			(((e) > MIIO_OT_ERR_CODE_MIN && (e) < MIIO_OT_ERR_CODE_MAX) ? (e) : MIIO_OT_ERR_CODE_MAX)


#define MIIO_OT_ERR_INFO_UNDEF_ERROR			"sys undefined error."
#define MIIO_OT_ERR_CODE_UNDEF_ERROR			MIIO_OT_ERR_CODE_L(0)

#define MIIO_OT_ERR_INFO_RESP_INVALID			"resp invalid."
#define MIIO_OT_ERR_CODE_RESP_INVALID			MIIO_OT_ERR_CODE_L(1)

#define MIIO_OT_ERR_INFO_TRYOUT					"try out."
#define MIIO_OT_ERR_CODE_TRYOUT					MIIO_OT_ERR_CODE_L(11)

#define MIIO_OT_ERR_INFO_BUSY					"busy."
#define MIIO_OT_ERR_CODE_BUSY					MIIO_OT_ERR_CODE_L(12)

#define MIIO_OT_ERR_INFO_OFFLINE				"offline."
#define MIIO_OT_ERR_CODE_OFFLINE				MIIO_OT_ERR_CODE_L(13)

#define MIIO_OT_ERR_INFO_NOT_SYNC				"not sync."
#define MIIO_OT_ERR_CODE_NOT_SYNC				MIIO_OT_ERR_CODE_L(14)

#define MIIO_OT_ERR_INFO_SERVICE_NOT_AVAILABLE	"service not available."
#define MIIO_OT_ERR_CODE_SERVICE_NOT_AVAILABLE	MIIO_OT_ERR_CODE_L(20)

#define MIIO_OT_ERR_INFO_REQ_ERROR				"req error."
#define MIIO_OT_ERR_CODE_REQ_ERROR				MIIO_OT_ERR_CODE_L(2600)

#define MIIO_OT_ERR_INFO_METHOD_INVALID			"method not found."
#define MIIO_OT_ERR_CODE_METHOD_INVALID			MIIO_OT_ERR_CODE_L(2601)

#define MIIO_OT_ERR_INFO_PARAM_INVALID			"invalid param."
#define MIIO_OT_ERR_CODE_PARAM_INVALID			MIIO_OT_ERR_CODE_L(2602)

/* ==================== ota error code ==================== */
/* ================== -33100 < x < -33000 ================= */
#define MIIO_OTA_ERR_CODE_MIN				(-33100)
#define MIIO_OTA_ERR_CODE_MAX				(-33000)
#define MIIO_OTA_ERR_CODE_R(e)				(MIIO_OTA_ERR_CODE_MIN+abs(e))
#define MIIO_OTA_ERR_CODE_L(e)				(MIIO_OTA_ERR_CODE_MAX-abs(e))
#define MIIO_OTA_ERR_CODE_FILTER(e)			(((e) > MIIO_OTA_ERR_CODE_MIN && (e) < MIIO_OTA_ERR_CODE_MAX) ? (e) : MIIO_OTA_ERR_CODE_MAX)
#define MIIO_OTA_ERR_CODE_CHECK(e)			((e) > MIIO_OTA_ERR_CODE_MIN && (e) < MIIO_OTA_ERR_CODE_MAX)

#define MIIO_OTA_ERR_INFO_DOWN				"down error"
#define MIIO_OTA_ERR_CODE_DOWN				MIIO_OTA_ERR_CODE_L(1)

#define MIIO_OTA_ERR_INFO_DNS				"dns error"
#define MIIO_OTA_ERR_CODE_DNS				MIIO_OTA_ERR_CODE_L(2)

#define MIIO_OTA_ERR_INFO_CONNECT			"connect error"
#define MIIO_OTA_ERR_CODE_CONNECT			MIIO_OTA_ERR_CODE_L(3)

#define MIIO_OTA_ERR_INFO_DISCONNECT		"disconnect"
#define MIIO_OTA_ERR_CODE_DISCONNECT		MIIO_OTA_ERR_CODE_L(4)

#define MIIO_OTA_ERR_INFO_INSTALL			"install error"
#define MIIO_OTA_ERR_CODE_INSTALL			MIIO_OTA_ERR_CODE_L(5)

#define MIIO_OTA_ERR_INFO_CANCEL			"cancel"
#define MIIO_OTA_ERR_CODE_CANCEL			MIIO_OTA_ERR_CODE_L(6)

#define MIIO_OTA_ERR_INFO_LOW_ENERGY		"low energy"
#define MIIO_OTA_ERR_CODE_LOW_ENERGY		MIIO_OTA_ERR_CODE_L(7)

/* ==================== user error code ==================== */
/* ================== -10000 < x < -5000 =================== */
#define MIIO_USER_ERR_CODE_MIN					(-10000)
#define MIIO_USER_ERR_CODE_MAX					(-5000)
#define MIIO_USER_ERR_CODE_R(e)					(MIIO_USER_ERR_CODE_MIN+abs(e))
#define MIIO_USER_ERR_CODE_L(e)					(MIIO_USER_ERR_CODE_MAX-abs(e))

#define MIIO_USER_ERR_CODE_FILTER(e)			(((e) >= MIIO_USER_ERR_CODE_MIN && (e) <= MIIO_USER_ERR_CODE_MAX) ? (e) : MIIO_USER_ERR_CODE_MIN)

//user defined error
#define MIIO_USER_ERR_INFO_UNDEF_ERROR			"user undefined error"
#define MIIO_USER_ERR_CODE_UNDEF_ERROR			MIIO_USER_ERR_CODE_R(0)

//user down command timeout
#define MIIO_USER_ERR_INFO_ACK_TIMEOUT			"user ack timeout"
#define MIIO_USER_ERR_CODE_ACK_TIMEOUT			MIIO_USER_ERR_CODE_R(1)

//user down command ack invalid
#define MIIO_USER_ERR_INFO_ACK_INVALID			"user ack invalid"
#define MIIO_USER_ERR_CODE_ACK_INVALID			MIIO_USER_ERR_CODE_R(2)

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


typedef  void* miio_handle_t;

typedef struct {
  size_t pload_len;
  void* pload;
}miio_cmd_delegate_arg_t;

typedef int (*miio_fp_cmd_delegate_ack_t)(void* handle, const char* pbuf, size_t buf_sz);

typedef enum _uart_error_t {
  UART_OK = 0,
  UART_DESTROY_ERROR = -1,
  UART_OPEN_ERROR = -2,
  UART_SET_ARRT_ERROR = -3,
  UART_SEND_ERROR = -4,
  UART_RECV_ACK_ERROR = -5,
  UART_RECV_ERROR = -6,
} uart_error_t;

/* buffer used to receive command string from wifi module */
#define CMD_STR_MAX_LEN             (800) //max 800 byte
/* max method name register with miio_cmd_method_register() function */
#define CMD_METHOD_LEN_MAX          (64)
/* buffer used to response command execute result */
#define RESULT_BUF_SIZE             CMD_STR_MAX_LEN

#define ACK_BUF_SIZE                CMD_STR_MAX_LEN
#define DATA_STRING_MAX_LENGTH      (800)

#define RINGBUFF_LEN                (1000) //USER_UART_RXBUF_SIZE

/* user uart configurations */
#define USER_UART_RXBUF_SIZE        (256)
#define USER_UART_TXBUF_SIZE        (256)

#endif
