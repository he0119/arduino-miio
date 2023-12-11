/**
 * @par     Copyright (c):
 *
 *    Copyright 2019 MIoT,MI
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef __ACTION_OPERATION_H__
#define __ACTION_OPERATION_H__

#include "arguments.h"

#define DID_MAX_LENGTH 128

typedef struct _action_operation {
  uint32_t siid;
  uint32_t aiid;
  uint32_t piid;
  int code;
  arguments_t *in;
  arguments_t *out;
} action_operation_t;

action_operation_t *action_operation_new(void);
void action_operation_delete(action_operation_t *thiz);

#endif /* __ACTION_OPERATION_H__ */
