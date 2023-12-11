/**
 * Copyright (C) 2013-2015
 *
 * @file   property_value.c
 *
 * @remark
 *
 */

#include "property_value.h"
#include "miio/miio_define.h"

property_value_t *property_value_new(void) {
  property_value_t *thiz = NULL;

  do {
    thiz = (property_value_t *)malloc(sizeof(property_value_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(property_value_t));
    thiz->format = PROPERTY_FORMAT_UNDEFINED;
  } while (false);

  return thiz;
}

property_value_t *property_value_new_string(const char *value) {
  property_value_t *thiz = property_value_new();
  if (thiz != NULL) {
    thiz->format = PROPERTY_FORMAT_STRING;
    strncpy(thiz->data.string.value, value, DATA_STRING_MAX_LENGTH);
    thiz->data.string.length = strlen(thiz->data.string.value);
  }

  return thiz;
}

property_value_t *property_value_new_integer(long value) {
  property_value_t *thiz = property_value_new();
  if (thiz != NULL) {
    thiz->format = PROPERTY_FORMAT_NUMBER;
    thiz->data.number.type = DATA_NUMBER_INTEGER;
    thiz->data.number.value.integerValue = value;
  }

  return thiz;
}

property_value_t *property_value_new_float(float value) {
  property_value_t *thiz = property_value_new();
  if (thiz != NULL) {
    thiz->format = PROPERTY_FORMAT_NUMBER;
    thiz->data.number.type = DATA_NUMBER_FLOAT;
    thiz->data.number.value.floatValue = value;
  }

  return thiz;
}

property_value_t *property_value_new_boolean(bool value) {
  property_value_t *thiz = property_value_new();
  if (thiz != NULL) {
    thiz->format = PROPERTY_FORMAT_BOOLEAN;
    thiz->data.boolean.value = value;
  }

  return thiz;
}

void property_value_delete(property_value_t *thiz) {
  switch (thiz->format) {
  case PROPERTY_FORMAT_BOOLEAN:
    break;

  case PROPERTY_FORMAT_STRING:
    break;

  case PROPERTY_FORMAT_NUMBER:
    break;

  default:
    break;
  }

  free(thiz);
}
