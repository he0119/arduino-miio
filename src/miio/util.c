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

#include "Arduino.h"

#include "util.h"

void str_n_cat(char *pDst, int n_str, ...) {
  va_list list;
  va_start(list, n_str);

  const char *p = NULL;

  while (n_str--) {
    p = va_arg(list, const char *);
    strncat(pDst, p, strlen(p));
  }

  va_end(list);
}

int str_cut_head_c(char *str, int str_len, char c) {
  int c_nums = 0;

  if (str_len == 0) str_len = strlen(str);

  for (int i = 0; i < str_len; i++) {
    if (str[i] != c) break;
    c_nums++;
  }

  if (c_nums) {
    for (int i = 0; i < str_len; i++) {
      if (i < str_len - c_nums)
        str[i] = str[i + c_nums];
      else
        str[i] = '\0';
    }
  }

  return c_nums;
}

int str_cut_tail_c(char *str, int str_len, char c) {
  int c_nums = 0;

  if (str_len == 0) str_len = strlen(str);

  for (int i = str_len - 1; i >= 0; i--) {
    if (str[i] != c) break;
    str[i] = '\0';
    c_nums++;
  }

  return c_nums;
}

// 转换为无符号数
// n表示读取数字字符的最大长度
uint32_t arch_atoun(const char *c, size_t n) {
  uint32_t dig = 0;
  const char *org = c;
  while (isdigit((int)*c) && (c - org < n)) {
    dig = dig * 10 + *c - '0';
    c++;
  }
  return dig;
}

double arch_atofn(const char *c, size_t n) {
  double val = 0.0;
  const char *c_end;

  if (c == NULL || n == 0) goto finish_exit;

  c_end = c + n;

  while (c < c_end && isspace((int)*c)) c++;

  if (c >= c_end) goto finish_exit;

  {
    int flag = 1;
    if (*c == '-') flag = -1;
    if (*c == '+' || *c == '-') c++;

    while (c < c_end && isdigit((int)*c)) {
      val = val * 10.0 + (*c - '0');
      c++;
    }

    double power = 1.0;
    if (c < c_end && *c == '.') {
      c++;
      while (c < c_end && isdigit((int)*c)) {
        val = val * 10.0 + (*c - '0');
        power *= 10.0;
        c++;
      }
    }
    val = (flag * val) / power;
  }

  if (c >= c_end) goto finish_exit;

  if (*c == 'e' || *c == 'E') {
    int flag = 1;
    int e = 0;
    if (++c < c_end) {
      if (*c == '-') flag = -1;
      if (*c == '+' || *c == '-') c++;
    }

    while (c < c_end && isdigit((int)*c)) {
      e = e * 10 + (*c - '0');
      c++;
    }

    if (flag == -1) {
      while (e-- > 0) val /= 10.0;
    } else {
      while (e-- > 0) val *= 10.0;
    }
  }

finish_exit:

  return val;
}

// 16进制数串转换为无符号数
uint32_t arch_axtou(const char *c) {
  uint32_t dig = 0;
  //	const char *org = c;
  while (isxdigit((int)*c)) {
    dig <<= 4;
    dig += 0x0000000F & hex_char_value((int)*c);
    c++;
  }
  return dig;
}

// 16进制数串转换为无符号数 n限制采样字符数
uint32_t arch_axtoun(const char *c, size_t n) {
  int dig = 0;
  const char *org = c;
  while (isxdigit((int)*c) && (c - org < n)) {
    dig = dig * 16 + hex_char_value((int)*c);
    c++;
  }
  return dig;
}

// 将HEX字符串转换为二进制数组
// in尺寸有限制
// out空间有限制
// in_len输出已处理输入长度
// 返回输出长度
// arch_axtobuf("12EF985xyz",out) 返回6，最后一个'5'被抛弃
size_t arch_axtobuf(
    const char *in,
    size_t in_size,
    uint8_t *out,
    size_t out_size,
    size_t *in_len) {
  const char *org_in = in;
  uint8_t *org_out = out;

  while (isxdigit((int)*in) && isxdigit((int)*(in + 1)) && // 输入不硌牙
         (in - org_in < in_size) && (out - org_out < out_size)) { // 不超限

    *out = (0x0F & hex_char_value((int)*(in + 1))) |
           (0xF0 & (hex_char_value((int)*in) << 4)); // 转换处理

    in += 2;
    out += 1; // 调整指针
  }

  if (in_len) *in_len = in - org_in;

  return out - org_out;
}
// 转换为无符号的数，n表示读取数字字符的最大长度。
uint64_t arch_atou64n(const char *c, size_t n) {
  uint64_t dig = 0;
  const char *org = c;
  while (isdigit((int)*c) && (c - org < n)) {
    dig = dig * 10 + *c - '0';
    c++;
  }
  return dig;
}

int snprintf_hex(
    char *buf, size_t buf_size, const uint8_t *data, size_t len, char style) {
  unsigned char spliter, uppercase;
  size_t n = 0;

  if (buf_size == 0) return 0;

  spliter = style & 0x7F;
  uppercase = (style & 0x80) ? 1 : 0;

  if (!isprint(spliter)) spliter = 0;

  for (size_t i = 0; i < len; i++) {
    // 若需要打印分隔符
    if (i < len - 1 && spliter)
      n += snprintf(
          buf + n,
          buf_size - n,
          (uppercase ? "%02X%c" : "%02x%c"),
          data[i],
          spliter);
    else
      n += snprintf(
          buf + n, buf_size - n, (uppercase ? "%02X" : "%02x"), data[i]);
    if (n >= buf_size) break;
  }

  return (n >= buf_size) ? (buf_size - 1) : n;
}

// 转换为无符号数
// n表示读取数字字符的最大长度
int64_t arch_atos64n(const char *c, size_t n) {
  if (*c == '-')
    return 0 - (int64_t)arch_atou64n(c + 1, n - 1);
  else
    return (int64_t)arch_atou64n(c, n);
}

// 转换为无符号数
// n表示读取数字字符的最大长度
int32_t arch_atoin(const char *c, size_t n) {
  if (*c == '-')
    return 0 - (int32_t)arch_atoun(c + 1, n - 1);
  else
    return (int32_t)arch_atoun(c, n);
}

int arch_u64toa(uint64_t data, char *c) {
  int num = 0, index;
  char tmp;

  while (data > 0) {
    c[num++] = data % 10 + '0';
    data /= 10;
  }
  c[num] = '\0';
  for (index = 0; index < (num >> 1); index++) {
    tmp = c[index];
    c[index] = c[num - 1 - index];
    c[num - 1 - index] = tmp;
  }

  return num;
}

int arch_s64toa(int64_t data, char *c) {
  if (data < 0) {
    c[0] = '-';
    data = -data;
    return (1 + arch_u64toa(data, c + 1));
  }

  return arch_u64toa(data, c);
}

bool str_all_c(uint8_t *str, uint8_t c, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (str[i] != c) return false;
  }
  return true;
}
