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

#ifndef __PROPERTY_DATA_H__
#define __PROPERTY_DATA_H__

#include "miio/miio_define.h"

typedef struct _data_boolean {
  bool value;
} data_boolean_t;

typedef struct _data_string {
  char value[CMD_BUF_SIZE + 1];
  uint32_t length;
} data_string_t;

typedef enum _data_number_type {
  DATA_NUMBER_INTEGER = 0,
  DATA_NUMBER_FLOAT = 1,
} data_number_type_t;

typedef struct _data_number_value {
  long integerValue;
  float floatValue;
} data_number_value_t;

typedef struct _data_number {
  data_number_type_t type;
  data_number_value_t value;
} data_number_t;

typedef union _property_data {
  data_boolean_t boolean;
  data_string_t string;
  data_number_t number;
} property_data_t;

#endif /* __PROPERTY_DATA_H__ */
