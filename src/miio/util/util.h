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

#ifndef __UTIL_H__
#define __UTIL_H__

#include "Arduino.h"

// ---------------- BITs -----------------

#define BIT_SET(flag, bit)                                                     \
  { (flag) |= (bit); }
#define BIT_CLEAR(flag, bit)                                                   \
  { (flag) &= ~(bit); }
#define BIT_IS_SET(flag, bit) ((flag) & (bit))

#define FLD_SET(flag, mask, val)                                               \
  {                                                                            \
    (flag) &= ~(mask);                                                         \
    (flag) |= (val);                                                           \
  }
#define FLD_GET(flag, mask) ((flag) & (mask))

// ---------------- Math -----------------
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

// ---------------- struct  -----------------
#define NELEMENTS(x) ((unsigned int)(sizeof(x) / sizeof(x[0])))
#define SIZEOF(s, m) ((unsigned int)(sizeof(((s *)0)->m)))
#define ATTRIBUTE_OFFSET(structure, elem)                                      \
  ((unsigned int)(&(((structure *)0)->elem)))
#define OFFSETOF(s, m) ((unsigned int)(&(((s *)0)->m)))
#define CONTAINER(structure, elem, elem_addr)                                  \
  ((structure *)((((unsigned int)(elem_addr))) -                               \
                 ATTRIBUTE_OFFSET(structure, elem)))

// ---------------- Ascii -----------------

#define in_range(c, lo, up) ((uint8_t)(c) >= lo && (uint8_t)(c) <= up)
#define dec_char_value(c) ((c) - '0')
#define hex_char_value(c)                                                      \
  ((uint8_t)(isdigit(c) ? dec_char_value(c)                                    \
                        : (((c) >= 'A' && (c) <= 'F') ? ((c) - 'A' + 10)       \
                                                      : ((c) - 'a' + 10))))

// ---------------- Endian -----------------
#define LONGVAL_BE(c0, c1, c2, c3)                                             \
  (((uint32_t)((uint8_t)(c0)) << 24) | (((uint32_t)((uint8_t)(c1))) << 16) |   \
   (((uint32_t)((uint8_t)(c2))) << 8) | ((uint32_t)((uint8_t)(c3))))
#define LONGVAL_LE(c0, c1, c2, c3)                                             \
  (((uint32_t)((uint8_t)(c3)) << 24) | (((uint32_t)((uint8_t)(c2))) << 16) |   \
   (((uint32_t)((uint8_t)(c1))) << 8) | ((uint32_t)((uint8_t)(c0))))

#define PTR2LONG_BE(ptr)                                                       \
  (((uint32_t)((uint8_t)(ptr)[0]) << 24) |                                     \
   (((uint32_t)((uint8_t)(ptr)[1])) << 16) |                                   \
   (((uint32_t)((uint8_t)(ptr)[2])) << 8) | ((uint32_t)((uint8_t)(ptr)[3])))
#define PTR2LONG_LE(ptr)                                                       \
  (((uint32_t)((uint8_t)(ptr)[3]) << 24) |                                     \
   (((uint32_t)((uint8_t)(ptr)[2])) << 16) |                                   \
   (((uint32_t)((uint8_t)(ptr)[1])) << 8) | ((uint32_t)((uint8_t)(ptr)[0])))

#define SHORTVAL_BE(c0, c1)                                                    \
  ((((uint16_t)((uint8_t)(c0))) << 8) | ((uint16_t)((uint8_t)(c1))))
#define SHORTVAL_LE(c0, c1)                                                    \
  ((((uint16_t)((uint8_t)(c1))) << 8) | ((uint16_t)((uint8_t)(c0))))

#define PTR2SHORT_BE(ptr) (((uint32_t)(ptr)[0] << 8) | ((uint32_t)(ptr)[1]))
#define PTR2SHORT_LE(ptr) (((uint32_t)(ptr)[1] << 8) | ((uint32_t)(ptr)[0]))

#define BYTE1(u32) ((uint8_t)((u32) & 0xFF))
#define BYTE2(u32) ((uint8_t)(((u32) >> 8) & 0xFF))
#define BYTE3(u32) ((uint8_t)(((u32) >> 16) & 0xFF))
#define BYTE4(u32) ((uint8_t)(((u32) >> 24) & 0xFF))

#define U32_ALIGN4(a) ((uint32_t)(((uint32_t)(a) + 3) & ~(uint32_t)(3)))
#define U32_ALIGN16(a) ((uint32_t)(((uint32_t)(a) + 15) & ~(uint32_t)(15)))

size_t memchcmp(const void *s, uint8_t c, size_t n);
int memcasecmp(const void *s1, const void *s2, size_t len);

int snprintf_hex(
    char *buf, size_t buf_size, const uint8_t *data, size_t len, char style);
#define snprintf_safe(buffer, size, format, ...)                               \
  ({                                                                           \
    int r__ = snprintf(buffer, size, format, ##__VA_ARGS__);                   \
    (r__ > (size)) ? (size) : r__;                                             \
  })

char *strrstr(const char *s1, const char *s2);
char *strnchr(const char *s1, char c, size_t n);
int str_cut_head_c(char *str, int str_len, char c);
int str_cut_tail_c(char *str, int str_len, char c);
bool str_all_c(uint8_t *str, uint8_t c, size_t len);

int64_t arch_atos64n(const char *c, size_t n);
uint64_t arch_atou64n(const char *c, size_t n);
int32_t arch_atoin(const char *c, size_t n);
uint32_t arch_atoun(const char *c, size_t n);
double arch_atofn(const char *c, size_t n);
uint32_t arch_axtou(const char *c);
uint32_t arch_axtoun(const char *c, size_t n);
size_t arch_axtobuf(
    const char *in,
    size_t in_size,
    uint8_t *out,
    size_t out_size,
    size_t *in_len);
int arch_u64toa(uint64_t data, char *c);
int arch_s64toa(int64_t data, char *c);

/**
 * @brief  link N string in order
 *
 * @param[in]  pDst: dst string to add new strings
 *             n_str: number of new string to be add
 *             ... : new strings' buffer
 *
 * @return     none
 */
void str_n_cat(char *pDst, int n_str, ...);

#endif
