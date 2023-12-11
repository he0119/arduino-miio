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

#ifndef __ARGUMENT_H__
#define __ARGUMENT_H__

#include "property_value.h"

typedef struct _argument {
  uint32_t piid;
  property_value_t *value;
} argument_t;

argument_t *argument_new(void);
void argument_delete(argument_t *thiz);

#endif /* __ARGUMENT_H__ */
