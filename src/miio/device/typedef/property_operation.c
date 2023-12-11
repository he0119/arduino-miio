/**
 * Copyright (C) 2013-2015
 *
 * @file   property_operation.c
 *
 * @remark
 *
 */
#include "miio/device/typedef/property_operation.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

property_operation_t *miio_property_operation_new(void) {
  property_operation_t *thiz = NULL;

  do {
    thiz = (property_operation_t *)malloc(sizeof(property_operation_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(property_operation_t));

    thiz->code = 0;
    thiz->siid = 0;
    thiz->piid = 0;
    thiz->value = NULL;
  } while (false);

  return thiz;
}

void miio_property_operation_delete(property_operation_t *thiz) {
  if (thiz->value != NULL) {
    property_value_delete(thiz->value);
  }

  free(thiz);
}
