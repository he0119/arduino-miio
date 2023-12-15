#include <SerialMIIO.h>
SerialMIIO miio(Serial2);

int status_update_flag;
bool on_status;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  // 可通过以下函数设置运行参数
  miio.setPollInterval(200);
  miio.setSerialTimeout(200);
  miio.setReceiveRetry(25);

  // log_level 范围为 0 ~ 5, 0 表示开启 debug 日志，5 表示关闭日志打印。
  miio.sendStrWaitAck("set_log_level 0");

  miio.begin("perdev.switch.004", "18031", "0001");

  miio.onPropertyGet(2, 1, P_2_1_On_doGet);
  miio.onPropertySet(2, 1, P_2_1_On_doSet);
  miio.onActionInvoke(2, 1, A_2_1_Toggle_doInvoke);
}

void P_2_1_On_doGet(property_operation_t *o) {
  // 这里需要读到属性真正的值
  o->value = property_value_new_boolean(on_status);
}

void P_2_1_On_doSet(property_operation_t *o) {
  // 判断数据格式是否正确，如果错误，返回代码: OPERATION_ERROR_VALUE
  if (o->value->format != PROPERTY_FORMAT_BOOLEAN) {
    o->code = OPERATION_ERROR_VALUE;
    return;
  }

  // 执行写操作: o->value->data.boolean.value;
  on_status = o->value->data.boolean.value;

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;

  // 上报状态
  status_update_flag = 0x01;
}

void A_2_1_Toggle_doInvoke(action_operation_t *o) {
  // 检查输入参数个数
  if (o->in->size != 0) {
    o->code = OPERATION_INVALID;
    return;
  }

  // 输出结果个数
  o->out->size = 0;

  // 执行动作;
  on_status = !on_status;

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;
}

void P_2_1_On_doChange(bool newValue) {
  if (miio.sendPropertyChanged(2, 1, property_value_new_boolean(newValue)) !=
      MIIO_OK) {
    Serial.println("send_property_changed failed!");
  }
}

void loop() {
  // 状态改变主动上报
  if ((status_update_flag & 0x0F) == 0x01) {
    P_2_1_On_doChange(on_status);
    status_update_flag = status_update_flag & 0xF0;
  }

  miio.loop();
}
