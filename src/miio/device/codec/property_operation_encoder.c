/**
 * Copyright (C) 2013-2015
 *
 * @file   property_operation_encoder.c
 *
 * @remark
 *
 */
#include "property_operation_encoder.h"
#include "miio/device/typedef/operation_code.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#undef TAG
#define TAG "property_operation_encoder"

#define ID_MAX_LEN 16    // max 16 byte
#define VALUE_MAX_LEN 16 // max 16 byte

/* ==============  Public functions  ============== */

int property_operation_encode_param(
    char *pbuf, size_t buf_sz, property_operation_t *opt, bool has_value) {
  int ret = MIIO_OK;

  if (pbuf == NULL || buf_sz <= 0 || opt == NULL) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  // TODO : 错误处理
  char siid_buf[ID_MAX_LEN] = {0};
  char piid_buf[ID_MAX_LEN] = {0};
  char code_buf[ID_MAX_LEN] = {0};
  snprintf(siid_buf, ID_MAX_LEN, "%d", opt->siid);
  snprintf(piid_buf, ID_MAX_LEN, "%d", opt->piid);
  snprintf(code_buf, ID_MAX_LEN, "%d", opt->code);
  str_n_cat(pbuf, 6, siid_buf, " ", piid_buf, " ", code_buf, " ");
  if (0 != opt->code) {
    str_n_cat(pbuf, 1, " ");
    return MIIO_OK;
  }

  if (!has_value) {
    switch (opt->value->format) {

    case PROPERTY_FORMAT_BOOLEAN:
      if (opt->value->data.boolean.value == false)
        strncat(pbuf, "false", strlen("false"));
      else
        strncat(pbuf, "true", strlen("true"));
      break;

    case PROPERTY_FORMAT_STRING:
      str_n_cat(pbuf, 3, "\"", opt->value->data.string.value, "\"");
      break;

    case PROPERTY_FORMAT_NUMBER:
      if (opt->value->data.number.type == DATA_NUMBER_INTEGER) {
        char integer_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            integer_buf,
            VALUE_MAX_LEN,
            "%d",
            (int)opt->value->data.number.value.integerValue);
        strncat(pbuf, integer_buf, strlen(integer_buf));
      }
      if (opt->value->data.number.type == DATA_NUMBER_FLOAT) {
        char integer_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            integer_buf,
            VALUE_MAX_LEN,
            "%f",
            (double)opt->value->data.number.value.floatValue);
        strncat(pbuf, integer_buf, strlen(integer_buf));
      }
      break;
    case PROPERTY_FORMAT_UNDEFINED:
      break;
    }
    str_n_cat(pbuf, 1, " ");
  }

  return ret;
}

int property_operation_encode_tail(char *pbuf, size_t buf_sz) {
  if (NULL == pbuf) {
    LOG_ERROR_TAG(TAG, "pbuf is NULL");
    return MIIO_ERROR_EMPTY;
  }
  if (buf_sz <= strlen(pbuf)) {
    LOG_ERROR_TAG(TAG, "params out of range");
    return MIIO_ERROR_SIZE;
  }

  strcat(pbuf, "\r");
  return MIIO_OK;
}
