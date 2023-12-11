/**
 * Copyright (C) 2013-2015
 *
 * @file   arguments.c
 *
 * @remark
 *
 */
#include "arguments.h"
#include "miio/miio_define.h"

arguments_t *arguments_new(void) {
  arguments_t *thiz = NULL;

  do {
    thiz = (arguments_t *)malloc(sizeof(arguments_t));
    if (thiz == NULL) {
      break;
    }

    memset(thiz, 0, sizeof(arguments_t));
    thiz->size = 0;
  } while (false);

  return thiz;
}

void arguments_delete(arguments_t *thiz) {
  int i = 0;
  if (thiz != NULL) {
    for (i = 0; i < thiz->size; ++i) {
      if (thiz->arguments[i].value != NULL) {
        property_value_delete(thiz->arguments[i].value);
        thiz->arguments[i].value = NULL;
      }
    }

    free(thiz);
  }

  return;
}
