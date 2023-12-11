/**
 * Copyright (C) 2013-2015
 *
 * @file   property_operation_decoder.c
 *
 * @remark
 *
 */

#include "property_operation_decoder.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#undef TAG
#define TAG "decoder"

static int miio_get_value(const char *pbuf, property_operation_t *opt) {
  int ret = MIIO_OK;
  property_value_t *new_value =
      (property_value_t *)malloc(sizeof(property_value_t));
  if (new_value == NULL) return NULL;
  memset(new_value, 0, sizeof(property_value_t));
  do {
    const char *temp = pbuf;

    if (*temp == '\"') {
      new_value->format = PROPERTY_FORMAT_STRING;
      new_value->data.string.length = strlen(temp);
      strncpy(
          new_value->data.string.value,
          temp + 1,
          new_value->data.string.length - 2);
    } else if (!strncmp(temp, "true", strlen("true"))) {
      new_value->format = PROPERTY_FORMAT_BOOLEAN;
      new_value->data.boolean.value = true;
    } else if (!strncmp(temp, "false", strlen("false"))) {
      new_value->format = PROPERTY_FORMAT_BOOLEAN;
      new_value->data.boolean.value = false;
    } else {
      // judge if integer value
      if (NULL == strchr(temp, '.')) {
        new_value->format = PROPERTY_FORMAT_NUMBER;
        new_value->data.number.type = DATA_NUMBER_INTEGER;
        new_value->data.number.value.integerValue = atoi(temp);
        new_value->data.number.value.floatValue = atof(temp);
      } else {
        new_value->format = PROPERTY_FORMAT_NUMBER;
        new_value->data.number.type = DATA_NUMBER_FLOAT;
        new_value->data.number.value.floatValue = atof(temp);
      }
    }

  } while (false);

  opt->value = new_value;

  return ret;
}

property_operation_t *miio_property_operation_decode(
    const char *pbuf, size_t buf_sz, uint32_t index, bool has_value) {
  int ret = MIIO_OK;
  property_operation_t *thiz = miio_property_operation_new();

  char *p_cmd_str = (char *)malloc(buf_sz);
  if (p_cmd_str == NULL) return NULL;
  memset(p_cmd_str, 0, buf_sz);
  // remove '\r' at the pbuf end
  memcpy(p_cmd_str, pbuf, buf_sz - 1);

  char *temp = NULL;
  temp = strtok(p_cmd_str, " "); /* pass string "down" */
  temp = strtok(NULL, " "); /* pass string "get_properties"/"set_properties" */

  // pass (index)s param
  int index_temp = index;
  if (has_value) {
    while (index_temp) {
      temp = strtok(NULL, " ");
      temp = strtok(NULL, " ");
      temp = strtok(NULL, " ");
      index_temp--;
    }
  } else {
    while (index_temp) {
      temp = strtok(NULL, " ");
      temp = strtok(NULL, " ");
      index_temp--;
    }
  }

  do {
    // get siid string
    temp = strtok(NULL, " ");
    thiz->siid = atoi(temp);

    // get piid string
    temp = strtok(NULL, " ");
    thiz->piid = atoi(temp);

    // 如果是 get 方法不需要获得参数中的 value
    if (!has_value) {
      break;
    }

    // get value string
    temp = strtok(NULL, " ");
    ret = miio_get_value(temp, thiz);

    if (ret != MIIO_OK) {
      break;
    }

  } while (false);

  if (ret != MIIO_OK) {
    miio_property_operation_delete(thiz);
    thiz = NULL;
  }

  if (NULL != p_cmd_str) {
    free(p_cmd_str);
  }

  return thiz;
}
