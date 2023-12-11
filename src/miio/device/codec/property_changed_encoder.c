/**
 * Copyright (C) 2013-2015
 *
 * @file   property_changed_encoder.c
 *
 * @remark
 *
 */
#include "property_changed_encoder.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#define ID_MAX_LEN 4
#define VALUE_MAX_LEN 16

int miio_changed_operation_encode_end(char out[], size_t size) {
  int ret = MIIO_OK;

  if (strlen(out) > size - 1) {
    ret = MIIO_ERROR;
    return ret;
  }

  str_n_cat(out, 1, "\r");

  return ret;
}

int miio_changed_operation_encode(
    property_operation_t *opt, char out[], size_t size) {
  memset(out, 0, size);
  str_n_cat(out, 2, "properties_changed", " ");

  char siid_buf[ID_MAX_LEN] = {0};
  char piid_buf[ID_MAX_LEN] = {0};

  snprintf(siid_buf, ID_MAX_LEN, "%d", opt->siid);
  snprintf(piid_buf, ID_MAX_LEN, "%d", opt->piid);

  str_n_cat(out, 4, siid_buf, " ", piid_buf, " ");

  switch (opt->value->format) {
  case PROPERTY_FORMAT_BOOLEAN: {
    if (opt->value->data.boolean.value == false) {
      str_n_cat(out, 2, "false", " ");
    } else {
      str_n_cat(out, 2, "true", " ");
    }
  } break;

  case PROPERTY_FORMAT_STRING: {
    str_n_cat(out, 2, opt->value->data.string.value, " ");
  } break;

  case PROPERTY_FORMAT_NUMBER: {
    if (opt->value->data.number.type == DATA_NUMBER_INTEGER) {
      char integer_buf[VALUE_MAX_LEN] = {0};
      snprintf(
          integer_buf,
          VALUE_MAX_LEN,
          "%d",
          (int)opt->value->data.number.value.integerValue);
      str_n_cat(out, 2, integer_buf, " ");
    } else if (opt->value->data.number.type == DATA_NUMBER_FLOAT) {
      char float_buf[VALUE_MAX_LEN] = {0};
      snprintf(
          float_buf,
          VALUE_MAX_LEN,
          "%f",
          opt->value->data.number.value.floatValue);
      str_n_cat(out, 2, float_buf, " ");
    }
  } break;

  case PROPERTY_FORMAT_UNDEFINED:
    break;
  }

  miio_changed_operation_encode_end(out, size);

  return MIIO_OK;
}
