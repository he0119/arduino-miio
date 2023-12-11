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
struct data_boolean_t {
  bool value;
};

struct data_string_t {
  char value[DATA_STRING_MAX_LENGTH + 1];
  uint32_t length;
};

struct data_number_value_t {
  long integerValue;
  float floatValue;
};

enum data_number_type_t {
  DATA_NUMBER_INTEGER = 0,
  DATA_NUMBER_FLOAT = 1,
};

struct data_number_t {
  data_number_type_t type;
  data_number_value_t value;
};

union property_data_t {
  data_boolean_t boolean;
  data_string_t string;
  data_number_t number;
};

#endif /* __PROPERTY_DATA_H__ */
