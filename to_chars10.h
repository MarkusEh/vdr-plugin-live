/*
  Copyright (C) Markus Ehrnsperger. All rights reserved.
  Licence: GNU General Public License version 3, with the GCC RUNTIME LIBRARY EXCEPTION

  Usage:
    use
      res = to_chars10(first, last, value);
    instead of
      res = std::to_chars(first, last, value, 10);
    same parameters, return values, ... . So, identical but faster

*/
#ifndef TO_CHARS10_H
#define TO_CHARS10_H

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace to_chars10_internal {

/*
  Naming schema

    char *itoaN_M(char *b, uintXX_t i):   ==================================
    - write between N and M digits to b.
    - if less than N digits are needed for i, left fill with '0'
    - return the one-past-the-end pointer of the digits written
    - even if b is returned (because i == 0 && N == 0), b[0] might change.
    - i < 10^M must be valid. This is not checked
*/

alignas(2) static const char digits_100[200] = {
  '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
  '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
  '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
  '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
  '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
  '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
  '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
  '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
  '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
  '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};

// ========  0-2 digits ========================================================
// max uint16_t 65535 -> large enough for 1-2 digits

inline char *itoa0_2(char *b, uint16_t i, bool write_ten) {
// write 0 to 2 chars to b. 0 <= i < 100
// if write_ten == true, write the higher(i/10) digit
// return b+write_ten
// note: if the lower(i%10) digit is needed, the caller has to increase b.
  const char *src = digits_100 + 2*i;
  *b = *src;
  b += write_ten;
  *b = *++src;
  return b;
}
inline char *itoa0_2(char *b, uint16_t i) {
// write 0 to 2 chars to b and return the new position, 0 <= i < 100
  return itoa0_2(b, i, i>9) + (i != 0);
}
inline char *itoa1_2(char *b, uint16_t i) {
// write 1 or 2 chars to b and return the new position, 0 <= i < 100
  return itoa0_2(b, i, i>9) + 1;
}

// ========  1-4 digits ========================================================
// max uint16_t 65535 -> large enough for 1-4 digits

inline char *itoa1_4(char *b, uint16_t i) {
// write 1 to 4 chars to b and return the new position, 0 <= i < 10^4
  uint16_t q = ((uint32_t)i * 5243U) >> 19;  // q = i/100;
  b = itoa0_2(b, q);
  return itoa0_2(b, i - q*100, i>9) + 1;
}
inline char *itoa3_4(char *b, uint16_t i) {
// write 3 to 4 chars to b and return the new position, 0 <= i < 10^4
// Left-fill with 0.
  uint16_t q = ((uint32_t)i * 5243U) >> 19;  // q = i/100;
  b = itoa1_2(b, q);
  memcpy(b, digits_100 + ((i - q*100) << 1), 2);
  return b+2;
}
inline char *itoa4_4(char *b, uint16_t i) {
// write exactly 4 chars to b. Left-fill with 0. i < 10^4
  uint16_t q = ((uint32_t)i * 5243U) >> 19; // q = i/100; i < 43699
  memcpy(b, digits_100 + (q << 1), 2);
  b += 2;
  memcpy(b, digits_100 + ((i - q*100) << 1), 2);
  return b+2;
}

// ========  1-8 digits ========================================================
// max uint32_t 4294967295  (10 digits)  -> large enough for 1-8 digits

inline char *itoa5_8(char *b, uint32_t i) {
// write 5 to 8 chars to b and return the new position, 0 <= i < 10^8
// Left-fill with 0.
  uint32_t q = i/10000;
  b = itoa1_4(b, q);
  return itoa4_4(b, i - q*10000);
}
inline char *itoa1_8(char *b, uint32_t i) {
// write 1 to 8 chars to b and return the new position,    0 <= i < 10^8
  uint32_t q = i/1000000;
  uint32_t j = i - q*1000000;
  b = itoa0_2(b, q);
  q = j/10000;
  j = j - q*10000;
  b = itoa0_2(b, q, i>99999) + (i>9999);
  q = (j * 5243U) >> 19; // q = j/100;
  b = itoa0_2(b, q, i>999) + (i>99);
  return itoa0_2(b, j - q*100, i>9) + 1;
}
inline char *itoa8_8(char *b, uint32_t i) {
// write exactly 8 chars to b. Left-fill with 0. i < 10^8  (required, but not checked)
  for (int j = 6; j > 0; j-=2) {
    uint32_t q = i/100;
    memcpy(b+j, digits_100 + ((i - q*100) << 1), 2);
    i = q;
  }
  memcpy(b, digits_100 + (i<< 1), 2);
  return b+8;
}

// ========  8-16 digits ========================================================

template<typename T> inline char *itoa9_16(char *b, T i) {
// write 9 to 16 chars to b and return the new position, 10^8 <= i < 10^16
  T q = i/100000000;
  b = itoa1_8(b, q);
  return itoa8_8(b, i - q*100000000);
}

// ========  implementation of itoa(char *b, T i), depending on type T  =========

// max uint8_t 256
template<typename T, std::enable_if_t<sizeof(T) == 1, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
// write 1 to 3 digits to b and return the new position
  if (i < 100) return itoa1_2(b, i);
  T q = i/100;  // 256/100 = 2
  *b++ = q + '0';
  memcpy(b, digits_100 + ((i - q*100) << 1), 2);
  return b+2;
}
// max uint16_t 65535 5 digits
template<typename T, std::enable_if_t<sizeof(T) == 2, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
// write 1 to 5 digits to b and return the new position
  if (i < 100) return itoa1_2(b, i);
  if (i < 10000) return itoa3_4(b, i);
  T q = i/10000;  // 65535/10000 = 6
  *b++ = q + '0';
  return itoa4_4(b, i - q*10000);
}
// max uint32_t 4294967295  (10 digits)
template<typename T, std::enable_if_t<sizeof(T) <= 4, bool> = true, std::enable_if_t<sizeof(T) >= 3, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
// write 1 to 10 digits to b and return the new position
  if (i < 100) return itoa1_2(b, i);
  if (i < 10000) return itoa3_4(b, i);
  if (i < 100000000) return itoa5_8(b, i);
  T q = i/100000000;  // 4294967295/100000000 = 42;
  b = itoa1_2(b, q);
  return itoa8_8(b, i - q*100000000);
}
// max uint64_t 18446744073709551615  (20 digits)
template<typename T, std::enable_if_t<sizeof(T) >= 5, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
// write 1 to 20 digits to b and return the new position
  if (i < 100) return itoa1_2(b, i);
  if (i < 10000) return itoa3_4(b, i);
  if (i < 100000000) return itoa5_8(b, i);
  if (i < 10000000000000000) return itoa9_16(b, i);
  T q = i/10000000000000000;  // 18446744073709551615/10000000000000000 = 1844
  b = itoa1_4(b, q);
  i = i - q*10000000000000000;   // now i up to 16 digits
  q = i/100000000;
  itoa8_8(b    , q);
  itoa8_8(b + 8, i - q*100000000);
  return b+16;
}
template<typename T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
// write 1 to 20 chars to b and return the new position
  typedef std::make_unsigned_t<T> TU;
  bool minus = i < 0;
  TU u = (TU)(i) - 2 * (TU)(i) * minus;
  *b = '-';
  return itoa(b + minus, u);
}

// ========  verify if the provided range is large enough               =========

// max_int[0] = 0
// max_int[N>0] = largest integer number that can be presented with N digits
static const uint64_t max_int[20] = {
  0,
  9,
  99,
  999,
  9999,
  99999,
  999999,
  9999999,
  99999999,
  999999999,
  9999999999,
  99999999999,
  999999999999,
  9999999999999,
  99999999999999,
  999999999999999,
  9999999999999999,
  99999999999999999,
  999999999999999999,
  9999999999999999999u };


// =====  max_chars_for_to_chars10(T value) ===============================
// return the maximum number of charaters that will be used by to_chars10
// for any value of data type T
template<typename T, std::enable_if_t<sizeof(T) == 1, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 3;  // 256
}
template<typename T, std::enable_if_t<sizeof(T) == 1, bool> = true, std::enable_if_t<std::is_signed_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 4;  // -128
}
template<typename T, std::enable_if_t<sizeof(T) == 2, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 5; // 65535
}
template<typename T, std::enable_if_t<sizeof(T) == 2, bool> = true, std::enable_if_t<std::is_signed_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 6;  // -32768
}
template<typename T, std::enable_if_t<sizeof(T) == 4, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 10; // 4294967295
}
template<typename T, std::enable_if_t<sizeof(T) == 4, bool> = true, std::enable_if_t<std::is_signed_v<T>, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 11; // -2147483648
}
template<typename T, std::enable_if_t<sizeof(T) == 8, bool> = true>
inline std::ptrdiff_t max_chars_for_to_chars10(T value) {
  return 20;
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
inline bool to_chars10_range_check(char* first, char* last, T value) {
// return true if [first, last) is large enough for value
  if (__builtin_expect(last - first >= max_chars_for_to_chars10(value), true)) return true;
  if (__builtin_expect(last <= first, false)) return false;
  if (value >= 0) {
//         last - first > 0
    return (uint64_t)value <= max_int[last - first];
  } else {
//         value != 0, && last - first - 1 < 19
    return (int64_t)value >= -(int64_t)max_int[last - first - 1];
  }
}

}  // end of namespace to_chars10_internal

#endif // TO_CHARS10_H
