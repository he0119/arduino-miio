/**
 * @author  MIoT
 * @date    2019
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

#ifndef __UART_COMMAND_H__
#define __UART_COMMAND_H__

#include "arch_os.h"
#include "list.h"
#include "miio/miio_define.h"

typedef int (*miio_fp_cmd_handle_t)(
    void *handle,
    miio_cmd_delegate_arg_t *req_arg,
    miio_fp_cmd_delegate_ack_t ack);

typedef struct _miio_cmd {
  list_head_t list;
  arch_os_mutex mutex;
  char method[CMD_METHOD_MAX_LEN];
  char loop_flag;
  miio_fp_cmd_handle_t cb;
} miio_cmd_t;

#endif
