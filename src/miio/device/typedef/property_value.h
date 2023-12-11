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

typedef struct _property_value {
  property_format_t format;
  property_data_t data;
} property_value_t;

property_value_t *property_value_new(void);
property_value_t *property_value_new_string(const char *value);
property_value_t *property_value_new_integer(long value);
property_value_t *property_value_new_float(float value);
property_value_t *property_value_new_boolean(bool value);
void property_value_delete(property_value_t *thiz);

#endif /* __PROPERTY_VALUE_H__ */
