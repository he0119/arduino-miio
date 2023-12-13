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

#ifndef __OPERATION_CODE_H__
#define __OPERATION_CODE_H__

/*
0	成功
1	接收到请求，但操作还没有完成
-4001	属性不可读
-4002	属性不可写
-4003	属性、方法、事件不存在
-4004	其他内部错误
-4005	属性 value 错误
-4006	方法 in 参数错误
-4007	did 错误
*/
typedef enum _operation_code {
  OPERATION_OK = 0,
  OPERATION_PENDING = 1,
  OPERATION_ERROR_CANNOT_READ = -4001,
  OPERATION_ERROR_CANNOT_WRITE = -4002,
  OPERATION_INVALID = -4003,
  OPERATION_ERROR_INTERNAL = -4004,
  OPERATION_ERROR_VALUE = -4005,
  OPERATION_ERROR_IN = -4006,
  OPERATION_ERROR_DID = -4007,
} operation_code_t;

#endif /* __OPERATION_CODE_H__ */
