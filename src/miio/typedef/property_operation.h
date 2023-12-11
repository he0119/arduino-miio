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

#ifndef __PROPERTY_OPERATION_H__
#define __PROPERTY_OPERATION_H__

#include "property_value.h"

enum property_operation_type {
  PROPERTY_OPERATION_GET = 0,
  PROPERTY_OPERATION_SET = 1,
};

struct property_operation_t {
  uint32_t siid;
  uint32_t piid;
  int code;
  property_value_t *value;
};

#endif /* __PROPERTY_OPERATION_H__ */
