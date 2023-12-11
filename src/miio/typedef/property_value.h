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

#ifndef __PROPERTY_VALUE_H__
#define __PROPERTY_VALUE_H__

#include "miio/miio_define.h"

#include "property_data.h"
#include "property_format.h"

struct property_value_t {
  property_format_t format;
  property_data_t data;

  property_value_t() {
    format = PROPERTY_FORMAT_UNDEFINED;
  }

  property_value_t(const char *value) {
    format = PROPERTY_FORMAT_STRING;
    strncpy(data.string.value, value, DATA_STRING_MAX_LENGTH);
    data.string.length = strlen(value);
  }

  property_value_t(long value) {
    format = PROPERTY_FORMAT_NUMBER;
    data.number.type = DATA_NUMBER_INTEGER;
    data.number.value.integerValue = value;
  }

  property_value_t(float value) {
    format = PROPERTY_FORMAT_NUMBER;
    data.number.type = DATA_NUMBER_FLOAT;
    data.number.value.floatValue = value;
  }

  property_value_t(bool value) {
    format = PROPERTY_FORMAT_BOOLEAN;
    data.boolean.value = value;
  }
};

#endif /* __PROPERTY_VALUE_H__ */
