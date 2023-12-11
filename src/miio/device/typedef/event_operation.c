/**
 * Copyright (C) 2013-2015
 *
 * @file   event_operation.c
 *
 * @remark
 *
 */
#include "miio/device/typedef/event_operation.h"
#include "miio/miio_define.h"

event_operation_t *event_operation_new(void) {
  event_operation_t *thiz = NULL;

  do {
    thiz = (event_operation_t *)malloc(sizeof(event_operation_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(event_operation_t));

    // snprintf(thiz->did, DID_MAX_LENGTH, "%llu", arch_get_did_ex());
    thiz->siid = 0;
    thiz->eiid = 0;

  } while (false);

  return thiz;
}

void event_operation_delete(event_operation_t *thiz) {
  if (thiz->arguments != NULL) {
    arguments_delete(thiz->arguments);
  }

  free(thiz);
}
