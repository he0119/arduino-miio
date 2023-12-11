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

#ifndef __PROPERTY_OPERATION_DECODER_H__
#define __PROPERTY_OPERATION_DECODER_H__

#include "miio/device/typedef/property_operation.h"

property_operation_t *miio_property_operation_decode(
    const char *pbuf, size_t buf_sz, uint32_t index, bool hasValue);

#endif /* __PROPERTY_OPERATION_DECODER_H__ */
