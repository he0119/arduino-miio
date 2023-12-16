/* perdev.switch.004 */

#include <SerialMIIO.h>
SerialMIIO miio(Serial2);

int statusUpdateFlag;

bool onStatus = false;
enum Fault {
  No_Faults = 0,
  Over_Temperature = 1,
  Overload = 2,
};
Fault faultStatus = No_Faults;
bool antiFlickerStatus = false;

#define LED 2

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, onStatus);

  // 可通过以下函数设置运行参数
  miio.setPollInterval(200);
  miio.setSerialTimeout(200);
  miio.setReceiveRetry(25);

  // log_level 范围为 0 ~ 5, 0 表示开启 debug 日志，5 表示关闭日志打印。
  // TODO: 以后可以提供一个函数来配置日志级别
  miio.sendStrWaitAck("set_log_level 0");

  // 设置设备的基本配置
  miio.begin("perdev.switch.004", "18031", "0001");

  // 注册属性和动作的回调函数
  miio.onPropertyGet(2, 1, P_2_1_On_doGet);
  miio.onPropertyGet(2, 3, P_2_3_Fault_doGet);
  miio.onPropertyGet(2, 4, P_2_4_AntiFlicker_doGet);
  miio.onPropertySet(2, 1, P_2_1_On_doSet);
  miio.onPropertySet(2, 4, P_2_4_AntiFlicker_doSet);
  miio.onActionInvoke(2, 1, A_2_1_Toggle_doInvoke);
}

/**
 * 格式: property_value_new_boolean(true 或 false)
 * 取值: true 或　false
 */
void P_2_1_On_doGet(property_operation_t *o) {
  // 这里需要读到属性真正的值
  o->value = property_value_new_boolean(onStatus);
}

/**
 * 格式: property_value_new_integer(整数)
 * 取值: 0(No Faults), 1(Over Temperature), 2(Overload)
 */
void P_2_3_Fault_doGet(property_operation_t *o) {
  o->value = property_value_new_integer(faultStatus);
}

/**
 * 格式: property_value_new_boolean(true 或 false)
 * 取值: true 或　false
 */
void P_2_4_AntiFlicker_doGet(property_operation_t *o) {
  o->value = property_value_new_boolean(antiFlickerStatus);
}

void P_2_1_On_doSet(property_operation_t *o) {
  // 判断数据格式是否正确，如果错误，返回代码: OPERATION_ERROR_VALUE
  if (o->value->format != PROPERTY_FORMAT_BOOLEAN) {
    o->code = OPERATION_ERROR_VALUE;
    return;
  }

  // 执行写操作: o->value->data.boolean.value;
  onStatus = o->value->data.boolean.value;
  digitalWrite(LED, onStatus);

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;

  // 上报状态
  statusUpdateFlag = 0x01;
}

void P_2_4_AntiFlicker_doSet(property_operation_t *o) {
  // 判断数据格式是否正确，如果错误，返回代码: OPERATION_ERROR_VALUE
  if (o->value->format != PROPERTY_FORMAT_BOOLEAN) {
    o->code = OPERATION_ERROR_VALUE;
    return;
  }

  // TODO: 执行写操作: o->value->data.boolean.value;
  antiFlickerStatus = o->value->data.boolean.value;

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;

  // 上报状态，通知app状态变化，统一APP修改、定时器触发后的上报机制
  statusUpdateFlag = 0x10;

  return;
}

void P_2_1_On_doChange(bool newValue) {
  if (miio.sendPropertyChanged(2, 1, property_value_new_boolean(newValue)) !=
      MIIO_OK) {
    Serial.println("send_property_changed failed!");
  }
}

void P_2_3_Fault_doChange(Fault newValue) {
  if (miio.sendPropertyChanged(2, 3, property_value_new_integer(newValue)) !=
      MIIO_OK) {
    Serial.println("send_property_changed failed!");
  }
}

void P_2_4_AntiFlicker_doChange(bool newValue) {
  if (miio.sendPropertyChanged(2, 4, property_value_new_boolean(newValue)) !=
      MIIO_OK) {
    Serial.println("send_property_changed failed!");
  }
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
  onStatus = !onStatus;
  digitalWrite(LED, onStatus);
  statusUpdateFlag = 0x01;

  // 如果成功，返回代码: OPERATION_OK
  o->code = OPERATION_OK;
}

void loop() {
  // 状态改变主动上报
  if ((statusUpdateFlag & 0x0F) == 0x01) {
    P_2_1_On_doChange(onStatus);
    statusUpdateFlag = statusUpdateFlag & 0xF0;
  }
  if ((statusUpdateFlag & 0xF0) == 0x10) {
    P_2_4_AntiFlicker_doChange(antiFlickerStatus);
    statusUpdateFlag = statusUpdateFlag & 0x0F;
  }

  // 串口数据处理循环
  miio.handle();
}
