/**
 * Copyright (C) 2013-2015
 *
 * @file   event_operation_encoder.c
 *
 * @remark
 *
 */
#include "event_operation_encoder.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#define ID_MAX_LEN 16
#define VALUE_MAX_LEN 64

/* ==============  Private functions  ============== */
#if 0
static void str_n_cat(char *pDst, int n_str, ...)
{
    va_list list;
    va_start(list, n_str);

    const char *p = NULL;

    while (n_str--) {
        p = va_arg(list, const char *);
        strncat(pDst, p, strlen(p));
    }

    va_end(list);
}
#endif

/* ==============  Public functions  ============== */

int miio_event_operation_encode_end(char out[], size_t size) {
  if (strlen(out) > size - 1) return MIIO_ERROR_PARAM;

  str_n_cat(out, 1, "\r");

  return MIIO_OK;
}

int miio_event_operation_encode(
    event_operation_t *opt, char out[], size_t size) {
  memset(out, 0, size);
  str_n_cat(out, 2, "event_occured", " ");

  char siid_buf[ID_MAX_LEN] = {0};
  char eiid_buf[ID_MAX_LEN] = {0};
  char piid_buf[ID_MAX_LEN] = {0};

  snprintf(siid_buf, ID_MAX_LEN, "%d", opt->siid);
  snprintf(eiid_buf, ID_MAX_LEN, "%d", opt->eiid);

  str_n_cat(out, 4, siid_buf, " ", eiid_buf, " ");

  for (int i = 0; i < opt->arguments->size; i++) {
    snprintf(piid_buf, ID_MAX_LEN, "%d", opt->arguments->arguments[i].piid);
    str_n_cat(out, 2, piid_buf, " ");

    switch (opt->arguments->arguments[i].value->format) {

    case PROPERTY_FORMAT_BOOLEAN: {
      if (opt->arguments->arguments[i].value->data.boolean.value == false) {
        str_n_cat(out, 2, "false", " ");
      } else {
        str_n_cat(out, 2, "true", " ");
      }
    } break;

    case PROPERTY_FORMAT_STRING: {
      str_n_cat(
          out, 2, opt->arguments->arguments[i].value->data.string.value, " ");
    } break;

    case PROPERTY_FORMAT_NUMBER: {
      if (opt->arguments->arguments[i].value->data.number.type ==
          DATA_NUMBER_INTEGER) {
        char integer_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            integer_buf,
            VALUE_MAX_LEN,
            "%d",
            (int)opt->arguments->arguments[i]
                .value->data.number.value.integerValue);
        str_n_cat(out, 2, integer_buf, " ");
      } else if (
          opt->arguments->arguments[i].value->data.number.type ==
          DATA_NUMBER_FLOAT) {
        char float_buf[VALUE_MAX_LEN] = {0};
        snprintf(
            float_buf,
            VALUE_MAX_LEN,
            "%f",
            opt->arguments->arguments[i].value->data.number.value.floatValue);
        str_n_cat(out, 2, float_buf, " ");
      }
    } break;

    case PROPERTY_FORMAT_UNDEFINED:
      break;
    }
  }

  miio_event_operation_encode_end(out, size);
  return MIIO_OK;
}
