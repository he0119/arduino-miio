/**
 * Copyright (C) 2013-2015
 *
 * @file   argument.c
 *
 * @remark
 *
 */
#include "argument.h"
#include "miio/miio_define.h"

argument_t *argument_new(void) {
  argument_t *thiz = NULL;

  do {
    thiz = (argument_t *)malloc(sizeof(argument_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(argument_t));
    thiz->piid = 0;
    thiz->value = NULL;
  } while (false);

  return thiz;
}

void argument_delete(argument_t *thiz) {
  if (thiz->value != NULL) {
    property_value_delete(thiz->value);
  }

  memset(thiz, 0, sizeof(argument_t));
}
