#include "uart_command_decoder.h"
#include "miio/miio_define.h"

int uart_comamnd_decoder(
    char *pbuf, uint32_t buf_sz, char *method, uint32_t *method_len) {
  char *cmd_buf = (char *)malloc(buf_sz);
  if (NULL == cmd_buf || NULL == pbuf) {
    goto error_exit;
  }

  strncpy(cmd_buf, pbuf, buf_sz);
  cmd_buf[buf_sz - 1] = SPACE_CHAR;

  char *temp = NULL;

  temp = strtok(cmd_buf, SPACE_STRING); /* pass "down" */
  if (NULL == temp) {
    goto error_exit;
  }

  /* judge if recieved "error\r" */
  if (!strncmp(
          temp,
          ERROR_STRING,
          strlen(ERROR_STRING) < strlen(temp) ? strlen(ERROR_STRING)
                                              : strlen(temp))) {
    goto error_exit;
  } else if (NULL == temp) {
    goto error_exit;
  }

  /* get "get_properties"/"set_properties"/"none"/"update_fw"/"action" */
  temp = strtok(NULL, SPACE_STRING);
  if (NULL == temp) {
    goto error_exit;
  }

normal_exit: {
  strncpy(method, temp, strlen(temp));
  *method_len = strlen(method);

  if (NULL != cmd_buf) {
    free(cmd_buf);
  }
  return MIIO_OK;
}

error_exit: {
  method = NULL;
  *method_len = 0;
  if (NULL != cmd_buf) {
    free(cmd_buf);
  }

  return MIIO_ERROR_PARAM;
}
}
