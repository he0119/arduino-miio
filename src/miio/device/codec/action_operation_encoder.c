/**
 * Copyright (C) 2013-2015
 *
 * @file   action_operation_encoder.c
 *
 * @remark
 *
 */
#include "action_operation_encoder.h"
#include "miio/device/typedef/action_operation.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#undef TAG
#define TAG "action_operation_encoder"

#define ID_MAX_LEN 16
#define VALUE_MAX_LEN 32

int action_operation_encode_param(
    char *pbuf, size_t buf_sz, action_operation_t *opt) {
  // TODO : 出错处理
  int ret = MIIO_OK;

  if (pbuf == NULL || buf_sz <= 0 || opt == NULL) {
    ret = MIIO_ERROR_PARAM;
    return ret;
  }

  char siid_buf[ID_MAX_LEN] = {0};
  memset(siid_buf, 0, ID_MAX_LEN);
  char aiid_buf[ID_MAX_LEN] = {0};
  memset(aiid_buf, 0, ID_MAX_LEN);
  char piid_buf[ID_MAX_LEN] = {0};
  memset(piid_buf, 0, ID_MAX_LEN);
  char code_buf[ID_MAX_LEN] = {0};
  memset(code_buf, 0, ID_MAX_LEN);

  snprintf(siid_buf, ID_MAX_LEN, "%d", opt->siid);
  snprintf(aiid_buf, ID_MAX_LEN, "%d", opt->aiid);
  snprintf(code_buf, ID_MAX_LEN, "%d", opt->code);

  if (0 != opt->code) {
    str_n_cat(pbuf, 1, " ");
    return MIIO_OK;
  }

  str_n_cat(pbuf, 6, siid_buf, " ", aiid_buf, " ", code_buf, " ");

  for (int i = 0; i < opt->out->size; i++) {

    snprintf(piid_buf, ID_MAX_LEN, "%d", opt->out->arguments[i].piid);
    str_n_cat(pbuf, 2, piid_buf, " ");

    switch (opt->out->arguments[i].value->format) {

    case PROPERTY_FORMAT_BOOLEAN:
      if (opt->out->arguments[i].value->data.boolean.value == false) {
        strncat(pbuf, "false", strlen("false"));
      } else {
        strncat(pbuf, "true", strlen("true"));
      }
      break;

    case PROPERTY_FORMAT_STRING:
      str_n_cat(
          pbuf, 3, "\"", opt->out->arguments[i].value->data.string.value, "\"");
      break;

    case PROPERTY_FORMAT_NUMBER:
      if (opt->out->arguments[i].value->data.number.type ==
          DATA_NUMBER_INTEGER) {
        char integer_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            integer_buf,
            VALUE_MAX_LEN,
            "%d",
            (int)opt->out->arguments[i].value->data.number.value.integerValue);
        strncat(pbuf, integer_buf, strlen(integer_buf));
      }
      if (opt->out->arguments[i].value->data.number.type == DATA_NUMBER_FLOAT) {
        char integer_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            integer_buf,
            VALUE_MAX_LEN,
            "%f",
            opt->out->arguments[i].value->data.number.value.floatValue);
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

int action_operation_encode_tail(char *pbuf, size_t buf_sz) {
  if (NULL == pbuf) {
    return MIIO_ERROR_EMPTY;
  }
  if (buf_sz <= strlen(pbuf)) {
    return MIIO_ERROR_SIZE;
  }

  strcat(pbuf, "\r");
  return MIIO_OK;
}
