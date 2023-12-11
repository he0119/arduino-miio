#include <MIIO.h>
MIIO miio(Serial);

void setup() {
  Serial.begin(115200);

  miio.setPollInterval(5000);
  miio.setSerialTimeout(2000);
  miio.setReceiveRetry(1);

  miio.begin("perdev.switch.004", "18031", "0001");

  miio.onPropertyGet(2, 3, P_2_3_Fault_doGet);
  miio.onPropertySet(2, 3, P_2_3_Fault_doSet);
  miio.onActionInvoke(2, 1, A_2_1_Toggle_doInvoke);
}

void P_2_3_Fault_doGet(property_operation_t *o) {
  // 这里需要读到属性真正的值
  o->value = property_value_new_integer(0);
}

void P_2_3_Fault_doSet(property_operation_t *o) {
  // 判断数据格式是否正确，如果错误，返回代码: OPERATION_ERROR_VALUE
  if (o->value->format != PROPERTY_FORMAT_BOOLEAN) {
    o->code = OPERATION_ERROR_VALUE;
    return;
  }

  // 执行写操作: o->value->data.boolean.value;
  Serial.println(o->value->data.boolean.value);

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;
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
  Serial.println("Toggle");

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;
}

void loop() {
  miio.loop();
}
