#include "action_operation_decoder.h"
#include "miio/miio_define.h"
#include "miio/util/util.h"

#define TAG "action_operation_decoder"

static int miio_action_get_arguments_in(
    action_operation_t *thiz, char *pbuf, uint32_t piid, int index) {
  int ret = MIIO_OK;

  do {
    char *temp = pbuf;
    thiz->in->arguments[index].piid = piid;
    thiz->in->size += 1;

    if (*temp == '\"') {
      thiz->in->arguments[index].value = property_value_new_string(temp);
    } else if (!strncmp(temp, "true", strlen("true"))) {
      thiz->in->arguments[index].value = property_value_new_boolean(true);
    } else if (!strncmp(temp, "false", strlen("false"))) {
      thiz->in->arguments[index].value = property_value_new_boolean(false);
    } else {
      // judge if integer value
      if (NULL == strchr(temp, '.')) {
        thiz->in->arguments[index].value =
            property_value_new_integer(atoi(temp));
      } else {
        thiz->in->arguments[index].value = property_value_new_float(atof(temp));
      }
    }
  } while (false);

  return ret;
}

action_operation_t *
action_operation_decode(const char *pbuf, size_t buf_sz, uint32_t index) {
  int ret = MIIO_OK;
  action_operation_t *thiz = action_operation_new();
  char *p_cmd_str = (char *)malloc(buf_sz);
  if (p_cmd_str == NULL) return NULL;
  memset(p_cmd_str, 0, buf_sz);
  memcpy(p_cmd_str, pbuf, buf_sz - 1); /* remove '\r' at the pbuf end */

  char *temp = NULL;
  temp = strtok(p_cmd_str, " "); /* pass "down" */
  temp = strtok(NULL, " ");      /* pass "action" */

  while (index) { /* pass (index * 4) parameters */
    temp = strtok(NULL, " ");
    temp = strtok(NULL, " ");
    temp = strtok(NULL, " ");
    temp = strtok(NULL, " ");
    index--;
  }

  do {
    // get siid
    temp = strtok(NULL, " ");
    thiz->siid = atoi(temp);

    // get aiid
    temp = strtok(NULL, " ");
    thiz->aiid = atoi(temp);

    // get piid & value
    int index = 0;
    while ((temp = strtok(NULL, " ")) != NULL) {
      uint32_t piid = atoi(temp);
      temp = strtok(NULL, " ");

      if (NULL == temp || (index + 1) > MAX_ARGUMENTS) {
        ret = MIIO_ERROR_PARAM;
        break;
      }

      miio_action_get_arguments_in(thiz, temp, piid, index++);
    }

    if (ret != MIIO_OK) {
      break;
    }

  } while (false);

  if (ret != MIIO_OK) {
    action_operation_delete(thiz);
    thiz = NULL;
  }

  if (NULL != p_cmd_str) {
    free(p_cmd_str);
  }

  return thiz;
}
