/**
 * Copyright (C) 2013-2015
 *
 * @file   action_operation.c
 *
 * @remark
 *
 */
#include "miio/device/typedef/action_operation.h"
#include "miio/miio_define.h"

action_operation_t *action_operation_new(void) {
  action_operation_t *thiz = NULL;

  do {
    thiz = (action_operation_t *)malloc(sizeof(action_operation_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(action_operation_t));

    thiz->code = 0;
    thiz->siid = 0;
    thiz->aiid = 0;

    thiz->in = arguments_new();
    if (thiz->in == NULL) {
      action_operation_delete(thiz);
      thiz = NULL;
      break;
    }

    thiz->out = arguments_new();
    if (thiz->out == NULL) {
      action_operation_delete(thiz);
      thiz = NULL;
      break;
    }
  } while (false);

  return thiz;
}

void action_operation_delete(action_operation_t *thiz) {
  if (thiz->in != NULL) {
    arguments_delete(thiz->in);
  }

  if (thiz->out != NULL) {
    arguments_delete(thiz->out);
  }

  free(thiz);
}
