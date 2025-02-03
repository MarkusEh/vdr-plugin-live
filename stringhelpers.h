/*
 * version 0.9.5
 * general string-helper functions
 * Note: currently, most up to date Version is in live!
 *
 * only depends on g++:
 *    -std=c++17 std:: standard headers
 *     on esyslog (from VDR)
 *     on "to_chars10.h"
 *
 * no other dependencies, so it can be easily included in any other header
 *
 *
*/
#ifndef __STRINGHELPERS_H
#define __STRINGHELPERS_H

#if !defined test_stringhelpers
#include "vdr/tools.h"
#endif
#include "to_chars10.h"
#include <cstdarg>
#include <string>
#include <string_view>
#include <string.h>
#include <regex>
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <locale>

#include <iostream>
#include <chrono>

// =========================================================
// =========================================================
// Chapter 0: **************************************
// methods for char *s, make sure that s==NULL is just an empty string
// =========================================================
// =========================================================

inline std::string charPointerToString(const char *s) {
  return s?s:std::string();
}
inline std::string charPointerToString(const unsigned char *s) {
  return s?reinterpret_cast<const char *>(s):std::string();
}
// challenge:
//   method with importing parameter std::string_view called with const char * = nullptr
//   undefined behavior, as std::string_view(nullptr) is undefined. In later c++ standard, it is even an abort
// solution:
//   a) be very careful, check const char * for nullptr before calling a method with std::string_view as import parameter
// or:
//   b) replace all std::string_view with cSv
//      very small performance impact if such a method if called with cSv
//      this will convert nullptr to empty cSv if called with const char *

// 2nd advantage of cSv: substr(pos) if pos > length: no dump, just an empty cSv as result

class utf8_iterator;
class cSv;
template<class T> inline T parse_int(cSv sv);
template<class T> inline T parse_unsigned(cSv sv);

class cSv: public std::string_view {
  friend class utf8_iterator;
  public:
    typedef typename std::string_view::size_type size_type;
    static const size_type npos = std::string_view::npos;
    cSv(): std::string_view() {}
    template<size_type N> cSv(const char (&s)[N]): std::string_view(s, N-1) {
//      std::cout << "cSv const char (&s)[N] " << s << "\n";
    }
    template<typename T, std::enable_if_t<std::is_same_v<T, const char*>, bool> = true>
    cSv(T s): std::string_view(charPointerToStringView(s)) {
//      std::cout << "cSv const char *s " << (s?s:"nullptr") << "\n";
    }
    template<typename T, std::enable_if_t<std::is_same_v<T, char*>, bool> = true>
    cSv(T s): std::string_view(charPointerToStringView(s)) {
//      std::cout << "cSv       char *s " << (s?s:"nullptr") << "\n";
    }
    cSv(const unsigned char *s): std::string_view(charPointerToStringView(reinterpret_cast<const char *>(s))) {}
    cSv(const char *s, size_type l): std::string_view(s, l) {}
    cSv(const unsigned char *s, size_type l): std::string_view(reinterpret_cast<const char *>(s), l) {}
    cSv(std::string_view sv): std::string_view(sv) {}
    cSv(const std::string &s): std::string_view(s) {}
    cSv substr(size_type pos) const { return (length() > pos)?cSv(data() + pos, length() - pos):cSv(); }
    cSv substr(size_type pos, size_type count) const { return (length() > pos)?cSv(data() + pos, std::min(length() - pos, count) ):cSv(); }
    size_type find2(char ch, size_type pos = 0) const { // as find, but return length() if ch is not found
      for (; pos < length() && (*this)[pos] != ch; ++pos);
      return pos;
    }
  private:
    static std::string_view charPointerToStringView(const char *s) {
      return s?std::string_view(s, strlen(s)):std::string_view();
    }
// =================================================
// *********   utf8  *****************
// =================================================
  public:
    int utf8CodepointIsValid(size_type pos) const {
// In case of invalid UTF8, return 0
// In case of invalid input, return -1 (pos  >= sc.length())
// otherwise, return number of characters for this UTF codepoint
// note: pos + number of characters <= sc.length(), this is checked (otherwise 0 is returned)

      if (pos >= length() ) return -1;
      static const uint8_t LEN[] = {2,2,2,2,3,3,4,0};

      int len = (((*this)[pos] & 0xC0) == 0xC0) * LEN[((*this)[pos] >> 3) & 7] + (((*this)[pos] | 0x7F) == 0x7F);
      if (len == 1) return 1;
      if (len + pos > length()) return 0;
      for (size_type k= pos + 1; k < pos + len; k++) if (((*this)[k] & 0xC0) != 0x80) return 0;
      return len;
    }
  private:
    size_type utf8ParseBackwards(size_type pos) const {
// pos <= s.length()! this is not checked
// return position of character before pos
// see also https://stackoverflow.com/questions/22257486/iterate-backwards-through-a-utf8-multibyte-string
      for (size_type i = pos; i > 0; ) {
        --i;
        if (((*this)[i] & 0xC0) != 0x80) return i;
// (s[i] & 0xC0) == 0x80 is true if bit 6 is clear and bit 7 is set
      }
      return 0;
    }
  public:
    utf8_iterator utf8_begin() const;
    utf8_iterator utf8_end() const;
    int compareLowerCase(cSv other, const std::locale &loc);
};

inline wint_t Utf8ToUtf32(const char *p, int len);
// iterator for utf8
class utf8_iterator {
    const cSv m_sv;
    cSv::size_type m_pos;
    mutable int m_len = -2;
    int get_len() const {
      if (m_len == -2) m_len = m_sv.utf8CodepointIsValid(m_pos);
      return m_len;
    }
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = wint_t;
    using difference_type = int;
    using pointer = const wint_t*;
    using reference = const wint_t&;

    explicit utf8_iterator(cSv sv, size_t pos): m_sv(sv) {
// note: if pos is not begin/end, pos will be moved back to a valid utf8 start point
//       i.e. to an ASCII (bit 7 not set) or and utf8 start byte (bit 6&7 set)
      if (pos == 0) { m_pos = 0; return; }
      if (pos >= sv.length() ) { m_pos = sv.length(); return; }
// to avoid a position in the middle of utf8:
      m_pos = sv.utf8ParseBackwards(pos+1);
    }
    utf8_iterator& operator++() {
      if (m_pos >= m_sv.length() ) return *this;
      int l = get_len();
      if (l > 0) m_pos += l; else ++m_pos;
      m_len = -2;
      return *this;
    }
    utf8_iterator& operator--() {
      if (m_pos == 0) return *this;
      size_t new_pos = m_sv.utf8ParseBackwards(m_pos);
      int new_len = m_sv.utf8CodepointIsValid(new_pos);
      if (new_pos + new_len == m_pos || m_pos - new_pos == 1) {
        m_pos = new_pos;
        m_len = new_len;
      } else {
// some invalid UTF8.
        --m_pos;
        m_len = -2;
      }
      return *this;
    }
    bool operator!=(utf8_iterator other) const { return m_pos != other.m_pos; }
    bool operator==(utf8_iterator other) const { return m_pos == other.m_pos; }
    wint_t codepoint() const {
      if (m_pos >= m_sv.length() ) return 0;
      int l = get_len();
      if (l <= 0) return '?'; // invalid utf8
      return Utf8ToUtf32(m_sv.data() + m_pos, l);
    }
    size_t pos() const {
// note: if this == end(), sv[m_pos] is invalid!!!
      return m_pos;
    }
    const wint_t operator*() const {
      return codepoint();
    }
};
inline utf8_iterator cSv::utf8_begin() const { return utf8_iterator(*this, 0); }
inline utf8_iterator cSv::utf8_end() const { return utf8_iterator(*this, length() ); }

inline int cSv::compareLowerCase(cSv other, const std::locale &loc) {
// compare strings case-insensitive
  utf8_iterator ls = utf8_begin();
  utf8_iterator rs = other.utf8_begin();
  for (; ls != utf8_end() && rs != other.utf8_end(); ++ls, ++rs) {
    wint_t  llc = std::tolower<wchar_t>(*ls, loc);
    wint_t  rlc = std::tolower<wchar_t>(*rs, loc);
    if ( llc < rlc ) return -1;
    if ( llc > rlc ) return  1;
  }
  if (rs != other.utf8_end() ) return -1;
  if (ls !=       utf8_end() ) return  1;
  return 0;
}


// =========================================================
// cStr: similar to cSv, but support c_str()
// never returns null pointer!
// always return pointer to zero terminated char array
// =========================================================

class cStr {
  public:
    cStr() {}
    cStr(const char *s) { if (s) m_s = s; }
    cStr(const unsigned char *s) { if (s) m_s = reinterpret_cast<const char *>(s); }
    cStr(const std::string &s): m_s(s.c_str()) {}
    operator const char*() const { return m_s; }
    const char *c_str() const { return m_s; }
    char *data() { return (char *)m_s; }
    size_t length() const { return strlen(m_s); }
    operator cSv() const { return cSv(m_s, strlen(m_s)); }
  private:
    const char *m_s = "";
};

// =========================================================
// =========================================================
// Chapter 1: UTF8 string utilities ****************
// =========================================================
// =========================================================

inline void stringAppendUtfCodepoint(std::string &target, wint_t codepoint) {
  if (codepoint <= 0x7F){
     target.push_back( (char) (codepoint) );
     return;
  }
  if (codepoint <= 0x07FF) {
     target.push_back( (char) (0xC0 | (codepoint >> 6 ) ) );
     target.push_back( (char) (0x80 | (codepoint & 0x3F)) );
     return;
  }
  if (codepoint <= 0xFFFF) {
     target.push_back( (char) (0xE0 | ( codepoint >> 12)) );
     target.push_back( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
     target.push_back( (char) (0x80 | ( codepoint & 0x3F)) );
     return;
  }
     target.push_back( (char) (0xF0 | ((codepoint >> 18) & 0x07)) );
     target.push_back( (char) (0x80 | ((codepoint >> 12) & 0x3F)) );
     target.push_back( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
     target.push_back( (char) (0x80 | ( codepoint & 0x3F)) );
     return;
}

inline int utf8CodepointIsValid(const char *p) {
// p must be zero terminated

// In case of invalid UTF8, return 0
// otherwise, return number of characters for this UTF codepoint
  static const uint8_t LEN[] = {2,2,2,2,3,3,4,0};

  int len = ((*p & 0xC0) == 0xC0) * LEN[(*p >> 3) & 7] + ((*p | 0x7F) == 0x7F);
  for (int k=1; k < len; k++) if ((p[k] & 0xC0) != 0x80) return 0;
  return len;
}
inline wint_t Utf8ToUtf32(const char *p, int len) {
// assumes, that uft8 validity checks have already been done. len must be provided. call utf8CodepointIsValid first
  static const uint8_t FF_MSK[] = {0xFF >>0, 0xFF >>0, 0xFF >>3, 0xFF >>4, 0xFF >>5, 0xFF >>0, 0xFF >>0, 0xFF >>0};
  wint_t val = *p & FF_MSK[len];
  for (int i = 1; i < len; i++) val = (val << 6) | (p[i] & 0x3F);
  return val;
}

inline wint_t getNextUtfCodepoint(const char *&p) {
// p must be zero terminated

// get next codepoint, and increment p
// 0 is returned at end of string, and p will point to the end of the string (0)
  if(!p || !*p) return 0;
  int l = utf8CodepointIsValid(p);
  if( l == 0 ) { p++; return '?'; }
  wint_t result = Utf8ToUtf32(p, l);
  p += l;
  return result;
}

// =========================================================
// =========================================================
// Chapter 3: Parse char* / string_view / string
// =========================================================
// =========================================================

// =========================================================
// whitespace ==============================================
// =========================================================
inline bool my_isspace(char c) {
// fastest
  return (c == ' ') || (c >=  0x09 && c <=  0x0d);
// (0x09, '\t'), (0x0a, '\n'), (0x0b, '\v'),  (0x0c, '\f'), (0x0d, '\r')
}

inline cSv remove_trailing_whitespace(cSv sv) {
// return a string_view with trailing whitespace from sv removed
// for performance: see remove_leading_whitespace
  for (std::string_view::size_type i = sv.length(); i > 0; ) {
    i = sv.find_last_not_of(' ', i-1);
    if (i == std::string_view::npos) return cSv(); // only ' '
    if (sv[i] > 0x0d || sv[i] < 0x09) return sv.substr(0, i+1);  // non whitespace found at i -> length i+1 !!!
  }
  return cSv();
}
inline cSv remove_leading_whitespace(cSv sv) {
// return a string_view with leading whitespace from sv removed
// for performance:
//   avoid changing sv: cSv &sv is much slower than cSv sv
//   don't use std::isspace or isspace: this is really slow ... 0.055 <-> 0.037
//   also avoid find_first_not_of(" \t\f\v\n\r";): way too slow ...
// definition of whitespace:
// (0x20, ' '), (0x09, '\t'), (0x0a, '\n'), (0x0b, '\v'),  (0x0c, '\f'), (0x0d, '\r')
// or:  (c == ' ') || (c >=  0x09 && c <=  0x0d);
// best performance: use find_first_not_of for ' ':
  for (size_t i = 0; i < sv.length(); ++i) {
    i = sv.find_first_not_of(' ', i);
    if (i == std::string_view::npos) return cSv(); // only ' '
    if (sv[i] > 0x0d || sv[i] < 0x09) return sv.substr(i);  // non whitespace found at i
  }
  return cSv();
}
// =========================================================
// parse string_view for int
// =========================================================

template<class T> inline T parse_int_check_error(cSv sv, cSv::size_type start, cSv::size_type end, T val, T returnOnError, const char *context) {
// check for severe error (no digit available)
  if (start == end) {
// severe error, no data (no digit)
    if (context)
      esyslog(PLUGIN_NAME_I18N ": ERROR, cannot convert \"%.*s\" to int/bool, context %s", (int)sv.length(), sv.data(), context);
    return returnOnError;
  }
  if (context) {
// check for other errors -> any non-whitespace after number?
    if (remove_trailing_whitespace(sv).length() != end)
      isyslog(PLUGIN_NAME_I18N ": WARNING, trailing characters after convertion from \"%.*s\" to int/bool, context %s", (int)sv.length(), sv.data(), context);
  }
  return val;
}
template<class T> inline T parse_int_overflow(cSv sv, T returnOnError, const char *context) {
  if (context)
    esyslog(PLUGIN_NAME_I18N ": ERROR, integer overflow converting \"%.*s\" to int/bool, context %s", (int)sv.length(), sv.data(), context);
  return returnOnError;
}

template<class T> inline T parse_unsigned_internal(cSv sv, T returnOnError = T(), const char *context = nullptr) {
// T can also be a signed data type
// But: result will always be >=0, except in case of error and returnOnError < 0
  static const T limit_10 = std::numeric_limits<T>::max() / 10;
  T val = 0;
  cSv::size_type start = 0;
  for (; start < sv.length() && std::isdigit(sv[start]); ++start) {
    if (val > limit_10) return parse_int_overflow<T>(sv, returnOnError, context);
    val *= 10;
    T addval = sv[start]-'0';
    if (val > std::numeric_limits<T>::max() - addval) return parse_int_overflow<T>(sv, returnOnError, context);
    val += addval;
  }
  return parse_int_check_error<T>(sv, 0, start, val, returnOnError, context);
}
template<class T> inline T parse_neg_internal(cSv sv, T returnOnError = T(), const char *context = nullptr) {
// sv[0] == '-' must be correct, this is not checked!!
// T must be signed, a negative value will be returned
  static const T limit_10 = std::numeric_limits<T>::min() / 10;
  T val = 0;
  cSv::size_type start = 1;
  for (; start < sv.length() && std::isdigit(sv[start]); ++start) {
    if (val < limit_10) return parse_int_overflow<T>(sv, returnOnError, context);
    val *= 10;
    T addval = sv[start]-'0';
    if (val < std::numeric_limits<T>::min() + addval) return parse_int_overflow<T>(sv, returnOnError, context);
    val -= addval;
  }
  return parse_int_check_error<T>(sv, 1, start, val, returnOnError, context);
}

template<class T> inline T parse_int(cSv sv) {
  if (sv.empty() ) return 0;
  if (!std::isdigit(sv[0]) && sv[0] != '-') {
    sv = remove_leading_whitespace(sv);
    if (sv.empty() ) return 0;
  }
  if (sv[0] != '-') return parse_unsigned_internal<T>(sv);
  return -parse_unsigned_internal<T>(sv.substr(1));
}

template<class T> inline T parse_unsigned(cSv sv) {
  if (sv.empty() ) return 0;
  if (!std::isdigit(sv[0])) sv = remove_leading_whitespace(sv);
  return parse_unsigned_internal<T>(sv);
}

template<class T> inline T parse_hex(cSv sv, size_t *num_digits = 0) {
  static const signed char hex_values[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
  T value = 0;
  const unsigned char *data = reinterpret_cast<const unsigned char *>(sv.data());
  const unsigned char *data_e = data + sv.length();
  for (; data < data_e; ++data) {
    signed char val = hex_values[*data];
    if (val == -1) break;
    value = value*16 + val;
  }
  if (num_digits) *num_digits = data - reinterpret_cast<const unsigned char *>(sv.data());
  return value;
}
// =========================================================
// split string at delimiter in two parts
// =========================================================

inline bool splitString(cSv str, cSv delim, size_t minLengh, cSv &first, cSv &second) {
// true if delim is part of str, and length of first & second >= minLengh
  for (std::size_t found = str.find(delim); found != std::string::npos; found = str.find(delim, found + 1)) {
    cSv first_guess = remove_trailing_whitespace(str.substr(0, found));
    if (first_guess.length() >= minLengh) {
// we found the first part. Is the second part long enough?
      cSv second_guess = remove_leading_whitespace(str.substr(found + delim.length()));
      if (second_guess.length() < minLengh) return false; // nothing found

      first = first_guess;
      second = second_guess;
      return true;
    }
  }
  return false; // nothing found
}

inline cSv SecondPart(cSv str, cSv delim, size_t minLengh) {
// return second part of split string if delim is part of str, and length of first & second >= minLengh
// otherwise, return ""
  cSv first, second;
  if (splitString(str, delim, minLengh, first, second)) return second;
  else return cSv();
}

inline cSv SecondPart(cSv str, cSv delim) {
// if delim is not in str, return ""
// Otherwise, return part of str after first occurrence of delim
//   remove leading blanks from result
  size_t found = str.find(delim);
  if (found == std::string::npos) return cSv();
  std::size_t ssnd;
  for(ssnd = found + delim.length(); ssnd < str.length() && str[ssnd] == ' '; ssnd++);
  return str.substr(ssnd);
}

// =========================================================
// =========================================================
// Chapter 4: convert data to cSv:
//   cToSv classes, with buffer containing text representation of data
// =========================================================
// =========================================================

// =========================================================
// integer and hex
// =========================================================

namespace stringhelpers_internal {

//  ==== itoaN ===================================================================
// itoaN: Template for fixed number of characters, left fill with 0
// note: i must fit in N digits, this is not checked!
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N == 0, bool> = true>
inline char* itoaN(char *b, T i) { return b; }

template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N == 1, bool> = true>
inline char* itoaN(char *b, T i) {
  *b = i + '0';
  return b+N;
}
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N == 2, bool> = true>
inline char* itoaN(char *b, T i) {
  memcpy(b, to_chars10_internal::digits_100 + (i << 1), 2);
  return b+N;
}
// max uint16_t 65535
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && (N == 3 || N == 4), bool> = true>
inline char* itoaN(char *b, T i) {
  uint16_t q = (static_cast<uint32_t>(i) * 5243U) >> 19; // q = i/100; i < 43699
  b = itoaN<N-2>(b, q);
  memcpy(b, to_chars10_internal::digits_100 + ((static_cast<uint16_t>(i) - q*100) << 1), 2);
  return b+2;
}
// max uint32_t 4294967295, sizeof(uint32_t) == 4
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N >= 5 && (N <= 9 || sizeof(T) <= 4), bool> = true>
inline char* itoaN(char *b, T i) {
  for (int j = N-2; j > 0; j-=2) {
    uint32_t q = static_cast<uint32_t>(i)/100;
    memcpy(b+j, to_chars10_internal::digits_100 + ((static_cast<uint32_t>(i) - q*100) << 1), 2);
    i = q;
  }
  itoaN<2-N%2>(b, i);
  return b+N;
}
// for uint64_t, sizeof(uint64_t) == 8
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N >= 10 && N != 18 && sizeof(T) >= 5, bool> = true>
inline char* itoaN(char *b, T i) {
  T q = i/100000000;
  b = itoaN<N-8>(b, q);
  return itoaN<8>(b, static_cast<uint32_t>(i - q*100000000));
}
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N == 18 && sizeof(T) >= 5, bool> = true>
inline char* itoaN(char *b, T i) {
  T q = i/1000000000;
  b = itoaN<N-9>(b, static_cast<uint32_t>(q));
  return itoaN<9>(b, static_cast<uint32_t>(i - q*1000000000));
}
//  ==== powN ===============================
template<uint8_t N>
inline typename std::enable_if_t<N == 0, uint64_t> powN() { return 1; }
template<uint8_t N>
inline typename std::enable_if_t<N <= 19 && N >= 1, uint64_t> powN() {
// return 10^N
  return powN<N-1>() * 10;
}

//  ==== itoa_min_width =====================
template<size_t N, typename T, std::enable_if_t<std::is_integral_v<T> && N == 0, bool> = true>
inline char* itoa_min_width(char *b, T i) {
  return to_chars10_internal::itoa(b, i);
}
template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N >= 1 && N <= 19, bool> = true>
inline char* itoa_min_width(char *b, T i) {
  if (i < powN<N>() ) return itoaN<N, T>(b, i);
  T q = i/powN<N>();
  b = to_chars10_internal::itoa(b, q);
  return itoaN<N, T>(b, i - q*powN<N>() );
}

template<size_t N, typename T, std::enable_if_t<std::is_unsigned_v<T> && N >= 20, bool> = true>
inline char* itoa_min_width(char *b, T i) {
// i < 10^20 is always true
  memset(b, '0', N-20);
  b += N-20;
  return itoaN<20, T>(b, i);
}
template<size_t N, typename T, std::enable_if_t<std::is_signed_v<T> && N >= 1, bool> = true>
inline char* itoa_min_width(char *b, T i) {
  typedef std::make_unsigned_t<T> TU;
  if (i >= 0) return itoa_min_width<N, TU>(b, (TU)i);
  *b = '-';
  return itoa_min_width<N-1, TU>(b + 1, ~(TU(i)) + (TU)1);
}

//  ==== addCharsHex ========================
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline T addCharsHex(char *buffer, size_t num_chars, T value) {
// sizeof(buffer) must be >= num_chars. This is not checked !!!
// value is written with num_chars chars
//   if value is too small -> left values filled with 0
//   if value is too high  -> the highest numbers are not written. This is not checked!
//           but, you can check: if the returned value is != 0, some chars are not written
  const char *hex_chars = "0123456789ABCDEF";
  for (char *be = buffer + num_chars -1; be >= buffer; --be, value /= 16) *be = hex_chars[value%16];
  return value;
  }
}

class cToSv {
  public:
    cToSv() {}
// not intended for copy
// you can copy the cSv of this class (from  operator cSv() )
    cToSv(const cToSv&) = delete;
    cToSv &operator= (const cToSv &) = delete;
// deleting this is good :)
// don't try to implement! Otherwise, users will expect something like
//  a = a.substr(0,3);
// and similar to work. Which is possible, implementing lost's of different cases.
// it's just not worth the offert. For normal =, users can write
//  a = a.erase(0).append(...)

    virtual ~cToSv() {}
    virtual operator cSv() const = 0;
};
inline std::ostream& operator<<(std::ostream& os, cToSv const& sv)
{
  return os << cSv(sv);
}

template<std::size_t N>
class cToSvHex: public cToSv {
  public:
template<typename T>
    cToSvHex(const T &value) { *this << value; }
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    cToSvHex &operator<<(T value) {
      stringhelpers_internal::addCharsHex(m_buffer, N, value);
      return *this;
    }
    operator cSv() const { return cSv(m_buffer, N); }
    char m_buffer[N];
  protected:
    cToSvHex() { }
};

// read files
class cOpen {
  public:
    cOpen(const char *pathname, int flags) {
      if (!pathname) return;
      m_fd = open(pathname, flags);
      checkError(pathname, errno);
    }
    cOpen(const char *pathname, int flags, mode_t mode) {
      if (!pathname) return;
      m_fd = open(pathname, flags, mode);
      checkError(pathname, errno);
    }
    operator int() const { return m_fd; }
    bool exists() const { return m_fd != -1; }
    ~cOpen() {
      if (m_fd != -1) close(m_fd);
    }
  private:
    void checkError(const char *pathname, int errno_l) {
      if (m_fd == -1) {
// no message for errno == ENOENT, the file just does not exist
        if (errno_l != ENOENT) esyslog(PLUGIN_NAME_I18N "cOpen::checkError, ERROR: open fails, errno %d, filename %s\n", errno_l, pathname);
      }
    }
    int m_fd = -1;
};
class cToSvFile: public cToSv {
  public:
    cToSvFile(cStr filename, size_t max_length = 0) { load(filename, max_length ); }
    operator cSv() const { return m_result; }
    char *data() { return m_s?m_s:m_empty; } // Is zero terminated
    const char *c_str() const { return m_s?m_s:m_empty; } // Is zero terminated
    operator cStr() const { return m_s; }
    bool exists() const { return m_exists; }
    ~cToSvFile() { std::free(m_s); }
  private:
    void load(const char *filename, size_t max_length) {
      for (int n_err = 0; n_err < 3; ++n_err) {
        if (load_int(filename, max_length) ) return;
        m_exists = false;
        std::free(m_s);
        m_s = nullptr;
        if (n_err < 3) sleep(1);
      }
      esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR: give up after 3 tries, filename %s", filename);
    }
    bool load_int(const char *filename, size_t max_length) {
// return false if an error occurred, and we should try again
      cOpen fd(filename, O_RDONLY);
      if (!fd.exists()) return true;
      struct stat buffer;
      if (fstat(fd, &buffer) != 0) {
        if (errno == ENOENT) return false;  // somehow strange, cOpen found the file, and fstat says it does not exist ... we try again
        esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR: in fstat, errno %d, filename %s\n", errno, filename);
        return true;
      }
// file exists, length buffer.st_size
      m_exists = true;
      if (buffer.st_size == 0) return true; // empty file
      size_t length = buffer.st_size;
      if (max_length != 0 && length > max_length) length = max_length;
      m_s = (char *) malloc((length + 1) * sizeof(char));  // add one. So we can add the 0 string terminator
      if (!m_s) {
        esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR out of memory, filename = %s, requested size = %zu\n", filename, length + 1);
        return true;
      }
      size_t num_read = 0;
      ssize_t num_read1 = 1;
      for (int num_errors = 0; num_errors < 3 && num_read < length; num_read += num_read1) {
        num_read1 = read(fd, m_s + num_read, length - num_read);
        if (num_read1 == 0) ++num_errors; // should not happen, because fstat reported file size >= length
        if (num_read1 == -1) {
          if (errno == ENOENT) return false;
          esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR: read failed, errno %d, filename %s, file size = %zu, num_read = %zu\n", errno, filename, (size_t)buffer.st_size, num_read);
          ++num_errors;
          num_read1 = 0;
          if (num_errors < 3) sleep(1);
        }
      }
      m_result = cSv(m_s, num_read);
      m_s[num_read] = 0;  // so data() returns a 0 terminated string
      if (num_read != length) {
        esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR: num_read = %zu, length = %zu, filename %s\n", num_read, length, filename);
      }
      return true;
    }
    bool m_exists = false;
    char *m_s = nullptr;
    cSv m_result;
    char m_empty[1] = "";
};
template<std::size_t N> class cToSvFileN: public cToSv {
// read up to N bytes from file
  public:
    cToSvFileN(cStr filename) { load(filename); }
    operator cSv() const { return m_result; }
    char *data() { return m_s; } // Is zero terminated
    const char *c_str() { return m_s; } // Is zero terminated
    operator cStr() const { return m_s; }
    bool exists() const { return m_exists; }
  private:
    void load(const char *filename) {
      cOpen fd(filename, O_RDONLY);
      if (!fd.exists()) return;
      m_exists = true;
      ssize_t num_read = read(fd, m_s, N);
      if (num_read == -1) {
        esyslog(PLUGIN_NAME_I18N "cToSvFile::load, ERROR: read fails, errno %d, filename %s\n", errno, filename);
        num_read = 0;
      }
      m_result = cSv(m_s, num_read);
      m_s[num_read] = 0;  // so data returns a 0 terminated string
    }
    bool m_exists = false;
    char m_s[N+1] = "";
    cSv m_result;
};

// =========================================================
// cToSvConcat =============================================
// =========================================================

// N: number of bytes in buffer on stack
template<size_t N = 255>
class cToSvConcat: public cToSv {
  public:
    template<typename... Args> cToSvConcat(Args&&... args) {
      concat(std::forward<Args>(args)...);
    }
    cToSvConcat &concat() { return *this; }
    template<typename T, typename... Args>
    cToSvConcat &concat(T &&n, Args&&... args) {
      *this << n;
      return concat(std::forward<Args>(args)...);
    }
    template<typename T>
    cToSvConcat &operator+=(T &&n) { return *this << n; }
// ========================
// overloads for concat
// char
    cToSvConcat &operator<<(char ch) {
      if (m_pos_for_append == m_be_data) ensure_free(1);
      *m_pos_for_append = ch;
      ++m_pos_for_append;
      return *this;
    }
// cSv, string, char * ...
    cToSvConcat &operator<<(cSv sv) { return append(sv); }
    template<size_t M>
// "awrhjo!"
    cToSvConcat &operator<<(const char (&s)[M]) {
      if (m_pos_for_append + M-1 > m_be_data) ensure_free(M-1);
      memcpy(m_pos_for_append, s, M-1);
      m_pos_for_append += M-1;
      return *this;
    }
// bool
    template<typename T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
    cToSvConcat &operator<<(T b) { return *this << (char)('0'+b); }
// int
    template<typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, bool> = true>
    cToSvConcat &operator<<(T i) {
      if (!to_chars10_internal::to_chars10_range_check(m_pos_for_append, m_be_data, i)) ensure_free(20);
      m_pos_for_append = to_chars10_internal::itoa(m_pos_for_append, i);
      return *this;
    }

// ========================
// overloads for append. Should be compatible to std::string.append(...)
// ========================
    cToSvConcat &append(cSv sv) {
      if (sv.empty() ) return *this; // this check is required: documentation of std::memcpy: If either dest or src is an invalid or null pointer, the behavior is undefined, even if count is zero.
      if (m_pos_for_append + sv.length() > m_be_data) ensure_free(sv.length() );
      memcpy(m_pos_for_append, sv.data(), sv.length());
      m_pos_for_append += sv.length();
      return *this;
    }
    cToSvConcat &append(const char *s, size_t len) {
      if (!s) return *this;
      if (m_pos_for_append + len > m_be_data) ensure_free(len);
      memcpy(m_pos_for_append, s, len);
      m_pos_for_append += len;
      return *this;
    }
    cToSvConcat &append(size_t count, char ch) {
      if (m_pos_for_append + count > m_be_data) ensure_free(count);
      memset(m_pos_for_append, ch, count);
      m_pos_for_append += count;
      return *this;
    }

// =======================
// special appends
// =======================

// =======================
// appendInt:   append integer (with some format options)
template<size_t M, typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvConcat &appendInt(T i) {
// append integer with min widh M. Left fill with 0, if required.
      if (m_pos_for_append + std::max(M, (size_t)20) > m_be_data) ensure_free(std::max(M, (size_t)20));
      m_pos_for_append = stringhelpers_internal::itoa_min_width<M, T>(m_pos_for_append, i);
      return *this;
    }
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    cToSvConcat &appendHex(T value, int width = sizeof(T)*2) {
      if (m_pos_for_append + width > m_be_data) ensure_free(width);
      stringhelpers_internal::addCharsHex(m_pos_for_append, width, value);
      m_pos_for_append += width;
      return *this;
    }
template<typename T, std::enable_if_t<sizeof(T) == 16, bool> = true>
    cToSvConcat &appendHex(T value) {
      *this << value;
      return *this;
    }
// =======================
// append_utf8:     append utf8 codepoint
    cToSvConcat &append_utf8(wint_t codepoint) {
      if (m_pos_for_append + 4 > m_be_data) ensure_free(4);
      if (codepoint <= 0x7F) {
        *(m_pos_for_append++) = (char) (codepoint);
        return *this;
      }
      if (codepoint <= 0x07FF) {
        *(m_pos_for_append++) =( (char) (0xC0 | (codepoint >> 6 ) ) );
        *(m_pos_for_append++) =( (char) (0x80 | (codepoint & 0x3F)) );
        return *this;
      }
      if (codepoint <= 0xFFFF) {
          *(m_pos_for_append++) =( (char) (0xE0 | ( codepoint >> 12)) );
          *(m_pos_for_append++) =( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
          *(m_pos_for_append++) =( (char) (0x80 | ( codepoint & 0x3F)) );
        return *this;
      }
      *(m_pos_for_append++) =( (char) (0xF0 | ((codepoint >> 18) & 0x07)) );
      *(m_pos_for_append++) =( (char) (0x80 | ((codepoint >> 12) & 0x3F)) );
      *(m_pos_for_append++) =( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
      *(m_pos_for_append++) =( (char) (0x80 | ( codepoint & 0x3F)) );
      return *this;
    }
// =======================
// appendToLower
    cToSvConcat &appendToLower(cSv sv, const std::locale &loc) {
      for (auto it = sv.utf8_begin(); it != sv.utf8_end(); ++it) {
        append_utf8(std::tolower<wchar_t>(*it, loc));
      }
      return *this;
    }
// apend text. Before appending, replace all occurrences of substring with replacement
    cToSvConcat &appendReplace(cSv text, cSv substring, cSv replacement) {
      size_t pos = 0, found;
      while ( (found = text.find(substring, pos)) != std::string_view::npos) {
        append(text.data()+pos, found-pos);
        append(replacement);
        pos = found + substring.length();
      }
      append(text.data()+pos, text.length()-pos);
      return *this;
    }
// Replaces the characters in the range [begin() + pos, begin() + std::min(pos + count, size())) with sv
    cToSvConcat &replace(size_t pos, size_t count, cSv sv) {
      if (pos >= length() ) return append(sv);
      if (pos + count >= length() ) { m_pos_for_append = m_buffer + pos; return append(sv); }
      if (sv.length() != count) {
        if (sv.length() > count) ensure_free(sv.length() - count);
        memmove(m_buffer+pos+sv.length(), m_buffer+pos+count, length() - (pos+count));
        m_pos_for_append += sv.length() - count;
      }
      memcpy(m_buffer+pos, sv.data(), sv.length() );
      return *this;
    }
// Replaces all occurrences of substring after pos with replacement
    cToSvConcat &replaceAll(cSv substring, cSv replacement, size_t pos = 0) {
      while ( (pos = cSv(*this).find(substring, pos)) != std::string_view::npos) {
        replace(pos, substring.length(), replacement);
        pos += replacement.length();
      }
      return *this;
    }

// =======================
// appendFormated append formatted
// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with GCC 13.1
    template<typename... Args> cToSvConcat &appendFormated(const char *fmt, Args&&... args) {
      int needed = snprintf(m_pos_for_append, m_be_data - m_pos_for_append, fmt, std::forward<Args>(args)...);
      if (needed < 0) {
        esyslog(PLUGIN_NAME_I18N ": ERROR, cToScConcat::appendFormated needed = %d, fmt = %s", needed, fmt);
        return *this; // error in snprintf
      }
      if (needed < m_be_data - m_pos_for_append) {
        m_pos_for_append += needed;
        return *this;
      }
      ensure_free(needed + 1);
      needed = sprintf(m_pos_for_append, fmt, std::forward<Args>(args)...);
      if (needed < 0) {
        esyslog(PLUGIN_NAME_I18N ": ERROR, cToScConcat::appendFormated needed (2) = %d, fmt = %s", needed, fmt);
        return *this; // error in sprintf
      }
      m_pos_for_append += needed;
      return *this;
    }
// =======================
// appendDateTime: append date/time formatted with strftime
    cToSvConcat &appendDateTime(cStr fmt, const std::tm *tp) {
      size_t needed = std::strftime(m_pos_for_append, m_be_data - m_pos_for_append, fmt.c_str(), tp);
      if (needed == 0) {
        ensure_free(1024);
        needed = std::strftime(m_pos_for_append, m_be_data - m_pos_for_append, fmt.c_str(), tp);
        if (needed == 0) {
          esyslog(PLUGIN_NAME_I18N ": ERROR, cToSvConcat::appendDateTime needed = 0, fmt = %s", fmt.c_str());
          return *this; // we did not expect to need more than 1024 chars for the formatted time ...
        }
      }
      m_pos_for_append += needed;
      return *this;
    }
    cToSvConcat &appendDateTime(cStr fmt, time_t time) {
      if (!time) return *this;
      struct std::tm tm_r;
      if (localtime_r( &time, &tm_r ) == 0 ) {
        esyslog(PLUGIN_NAME_I18N ": ERROR, cToSvConcat::appendDateTime localtime_r = 0, fmt = %s, time = %lld", fmt.c_str(), (long long)time);
        return *this;
        }
      return appendDateTime(fmt, &tm_r);
    }
// =======================
// appendUrlEscaped
    cToSvConcat &appendUrlEscaped(cSv sv) {
      const char* reserved = " !#$&'()*+,/:;=?@[]\"<>\n\r\t\\%";
// in addition to the reserved URI charaters as defined here https://en.wikipedia.org/wiki/Percent-encoding
// also escape html characters \"<>\n\r so no additional html-escaping is required
// \ is escaped for easy use in strings where \ has a special meaning
      for (size_t pos = 0; pos < sv.length(); ++pos) {
        char c = sv[pos];
        if (strchr(reserved, c)) {
          concat('%');
          appendHex((unsigned char)c, 2);
        } else if ((unsigned char)c < ' ' || c == 127) {
          concat('?');  // replace control characters with ?
        } else {
          int l = sv.utf8CodepointIsValid(pos);
          if (l == 0) concat('?'); // invalid utf
          else {
            append(sv.data() + pos, l);
            pos += l-1;
          }
        }
      }
      return *this;
    }
// ========================
// get data
    operator cSv() const { return cSv(m_buffer, m_pos_for_append-m_buffer); }
    char *data() const { *m_pos_for_append = 0; return m_buffer; }
    size_t length() const { return m_pos_for_append-m_buffer; }
    char *begin() const { return m_buffer; }
    char *end() const { return m_pos_for_append; }
    const char *c_str() const { *m_pos_for_append = 0; return m_buffer; }
    char operator[](size_t i) const { return *(m_buffer + i); }
    operator cStr() const { return this->c_str(); }
// ========================
// others
    bool empty() const { return m_buffer == m_pos_for_append; }
    void clear() { m_pos_for_append = m_buffer; }
    cToSvConcat &erase(size_t index = 0) {
      m_pos_for_append = std::min(m_pos_for_append, m_buffer + index);
      return *this;
    }
    cToSvConcat &erase(size_t index, size_t count) {
      size_t l_length = length();
      if ((index >= l_length) | (count == 0) ) return *this;
      if (index + count >= l_length) {
        m_pos_for_append = m_buffer + index;
      } else {
        memmove(m_buffer+index, m_buffer+index + count, l_length - index - count);
        m_pos_for_append -= count;
      }
      return *this;
    }
    void reserve(size_t r) const { m_reserve = r; }
    virtual ~cToSvConcat() {
      if (m_buffer_allocated) free (m_buffer_allocated);
    }
  private:
    void ensure_free(size_t l) {
// make sure that l bytes can we written at m_pos_for_append
      if (m_pos_for_append + l <= m_be_data) return;
      size_t current_length = length();
      size_t new_buffer_size = std::max(2*current_length + l + 200, m_reserve);
      if (!m_buffer_allocated) {
        m_buffer_allocated = (char *) std::malloc(new_buffer_size);
        if (!m_buffer_allocated) throw std::bad_alloc();
        memcpy(m_buffer_allocated, m_buffer_static, current_length);
      } else {
        m_buffer_allocated = (char *) std::realloc(m_buffer_allocated, new_buffer_size);
        if (!m_buffer_allocated) throw std::bad_alloc();
      }
      m_be_data = m_buffer_allocated + new_buffer_size - 1;
      m_buffer = m_buffer_allocated;
      m_pos_for_append = m_buffer + current_length;
    }
    char  m_buffer_static[N+1];
    char *m_buffer_allocated = nullptr;
    char *m_buffer = m_buffer_static;
  protected:
    char *m_pos_for_append = m_buffer;
    char *m_be_data = m_buffer + sizeof(m_buffer_static) - 1; // [m_buffer, m_be_data) is available for data.
// It must be possible to write the 0 terminator to m_be_data: *m_be_data = 0.
// m_pos_for_append <= m_be_data: must be always ensured.
//   m_be_data - m_pos_for_append: Number of bytes available for write
  private:
    mutable size_t m_reserve = 1024;
};

template<size_t N=0>
class cToSvInt: public cToSvConcat<std::max(N, (size_t)20)> {
  public:
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvInt(T i) {
      this->m_pos_for_append = stringhelpers_internal::itoa_min_width<N>(this->m_pos_for_append, i);
    }
};
template<std::size_t N = 255> 
class cToSvToLower: public cToSvConcat<N> {
  public:
    cToSvToLower(cSv sv, const std::locale &loc) {
      this->reserve(sv.length() + 5);
      this->appendToLower(sv, loc);
    }
};

template<std::size_t N = 255> 
class cToSvFormated: public cToSvConcat<N> {
  public:
// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with GCC 13.1
    template<typename... Args> cToSvFormated(const char *fmt, Args&&... args) {
      this->appendFormated(fmt, std::forward<Args>(args)...);
    }
};
class cToSvDateTime: public cToSvConcat<255> {
  public:
    cToSvDateTime(cStr fmt, time_t time) {
      this->appendDateTime(fmt, time);
    }
};
template<std::size_t N = 255>
class cToSvUrlEscaped: public cToSvConcat<N> {
  public:
    cToSvUrlEscaped(cSv sv) {
      this->appendUrlEscaped(sv);
    }
};

template<std::size_t N = 255>
class cToSvReplace: public cToSvConcat<N> {
  public:
    cToSvReplace(cSv text, cSv substring, cSv replacement) {
      this->appendReplace(text, substring, replacement);
    }
};

// =========================================================
// =========================================================
// stringAppend: for std::string & cToSvConcat
// =========================================================
// =========================================================

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
inline void stringAppend(std::string &str, T i) {
  char buf[20]; // unsigned int 64: max. 20. (18446744073709551615) signed int64: max. 19 (+ sign)
  str.append(buf, to_chars10_internal::itoa(buf, i) - buf);
}
template<std::size_t N, typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
inline void stringAppend(cToSvConcat<N> &s, T i) {
  s.concat(i);
}

// =========================================================
// =========== stringAppend ==  for many data types
// =========================================================

// strings
inline void stringAppend(std::string &str, const char *s) { if(s) str.append(s); }
inline void stringAppend(std::string &str, const std::string &s) { str.append(s); }
inline void stringAppend(std::string &str, std::string_view s) { str.append(s); }

template<typename T, typename U, typename... Args>
void stringAppend(std::string &str, const T &n, const U &u, Args&&... args) {
  stringAppend(str, n);
  stringAppend(str, u, std::forward<Args>(args)...);
}

// =========================================================
// =========== lexical_cast:
// =========== convert strings (cSv) to other data types
// =========================================================

/*
// 1: Make the best guess what the converted target might have to look like, based on sv
// 2: If this is not possible:
//      return returnOnError. If context is provided, write esyslog ERROR message
// 3: Otherwise:
//      in case of unexpected values in sv, if context is provided, write isyslog WARNING message
//      note: any non-whitespace after the data is considered as unexpected values
//      return the best gues (see 1).
*/

// trivial (to cSv, std::string, ...)
template<class T, std::enable_if_t<std::is_same_v<T, cSv>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) { return sv; }
template<class T, std::enable_if_t<std::is_same_v<T, std::string>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) { return static_cast<T>(sv); }

// unsigned integer
template<class T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true, std::enable_if_t<!std::is_same_v<T, bool>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) {
  cSv no_ws = remove_leading_whitespace(sv);
  return parse_unsigned_internal<T>(no_ws, returnOnError, context);
}

// signed integer
template<class T, std::enable_if_t<std::is_signed_v<T>, bool> = true, std::enable_if_t<!std::is_same_v<T, bool>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) {
  cSv no_ws = remove_leading_whitespace(sv);
  if (!no_ws.empty() && no_ws[0] == '-')
    return parse_neg_internal<T>(no_ws, returnOnError, context);
  else
    return parse_unsigned_internal<T>(no_ws, returnOnError, context);
}

// bool
template<class T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) {
  long long int i = lexical_cast<long long int>(sv, std::numeric_limits<long long int>::max(), context);
  if (i == std::numeric_limits<long long int>::max()) {
    i = lexical_cast<long long int>(sv, -1, context);
    if (i == -1) return returnOnError; // esyslog already written by lexical_cast<long long int>
  }
  if (context && (i > 1 || i < 0))
    isyslog(PLUGIN_NAME_I18N ": WARNING, converted \"%.*s\" to bool, but had to guess, context %s", (int)sv.length(), sv.data(), context);
  return i;
}
// =========================================================
// =========================================================
// Chapter 5: change string: mainly: append to string
// =========================================================
// =========================================================

inline void StringRemoveTrailingWhitespace(std::string &str) {
  str.erase(remove_trailing_whitespace(str).length());
}

inline int stringAppendAllASCIICharacters(std::string &target, const char *str) {
// append all characters > 31 (signed !!!!). Unsigned: 31 < character < 128
// return number of appended characters
  int i = 0;
  for (; reinterpret_cast<const signed char*>(str)[i] > 31; i++);
  target.append(str, i);
  return i;
}
inline void stringAppendRemoveControlCharacters(std::string &target, const char *str) {
// we replace control characters with " " and invalid UTF8 with "?"
// and remove trailing whitespace
  for(;;) {
    str += stringAppendAllASCIICharacters(target, str);
    wint_t cp = getNextUtfCodepoint(str);
    if (cp == 0) { StringRemoveTrailingWhitespace(target); return; }
    if (cp > 31) stringAppendUtfCodepoint(target, cp);
    else target.append(" ");
  }
}
inline void stringAppendRemoveControlCharactersKeepNl(std::string &target, const char *str) {
  for(;;) {
    str += stringAppendAllASCIICharacters(target, str);
    wint_t cp = getNextUtfCodepoint(str);
    if (cp == 0) { StringRemoveTrailingWhitespace(target); return; }
    if (cp == '\n') { StringRemoveTrailingWhitespace(target); target.append("\n"); continue; }
    if (cp > 31) stringAppendUtfCodepoint(target, cp);
    else target.append(" ");
  }
}

// =========================================================
// =========== concat      =================================
// =========================================================

// create a string with "exactly" the required capacity (call reserve() for that)
// note: cToSvConc has a better performance, so use
//   concat only if such a string is required
//   e.g. the string is member of your class
// otherwise, use cToSvConcat

inline size_t length_csv(cSv s1) { return s1.length(); }
template<typename... Args>
inline size_t length_csv(cSv s1, Args&&... args) {
  return s1.length() + length_csv(std::forward<Args>(args)...);
}
inline void append_csv(std::string &str, cSv s1) { str.append(s1); }
template<typename... Args>
inline void append_csv(std::string &str, cSv s1, Args&&... args) {
  str.append(s1);
  append_csv(str, std::forward<Args>(args)...);
}
template<typename... Args>
inline std::string concat(Args&&... args) {
  std::string result;
// yes, reserve improves performance. Yes, I tested: 0.17 -> 0.31
// also tested with reserve(200); -> (almost) no performance improvement
  result.reserve(length_csv(std::forward<Args>(args)...));
  append_csv(result, std::forward<Args>(args)...);
  return result;
}

// =========================================================
// parse string_view for XML
// =========================================================

class cSubstring{
  public:
    cSubstring(size_t pos_start, size_t len):
      m_pos_start(pos_start), m_len(len) {};
    cSubstring():
      m_pos_start(std::string::npos), m_len(0) {};
    bool found() const { return m_pos_start != std::string::npos; }
    cSv substr(cSv sv) const { return found()?sv.substr(m_pos_start, m_len):cSv(); }
template<std::size_t N> cSubstring substringInXmlTag(cSv sv, const char (&tag)[N]);
template <size_t N>
    cToSvConcat<N> &erase(cToSvConcat<N> &target, size_t tag_len) {
      if (found() ) target.erase(m_pos_start-tag_len-2, m_len+2*tag_len+5);
      return target;
    }
    std::string &erase(std::string &target, size_t tag_len) {
      if (found() ) target.erase(m_pos_start-tag_len-2, m_len+2*tag_len+5);
      return target;
    }
template <size_t N>
    cToSvConcat<N> &replace(cToSvConcat<N> &target, cSv sv) {
      if (found() ) target.replace(m_pos_start, m_len, sv);
      return target;
    }
    std::string &replace(std::string &target, cSv sv) {
      if (found() ) target.replace(m_pos_start, m_len, sv);
      return target;
    }
  private:
    size_t m_pos_start;
    size_t m_len;
};
template<std::size_t N> inline
cSubstring substringInXmlTag(cSv sv, const char (&tag)[N]) {
// very simple XML parser
// if sv contains <tag>...</tag>, ... is returned (part between the outermost XML tags is returned).
// there is no error checking, like <tag> is more often in sv than </tag>, ...

// N == strlen(tag) + 1. It includes the 0 terminator ...
// strlen(startTag) = N+1; strlen(endTag) = N+2. Sums to 2N+3
  if (N < 1 || sv.length() < 2*N+3) return cSubstring();
// create <tag>
  cToSvConcat<N+2> tagD("<<", tag, ">");
  size_t pos_start = sv.find(cSv(tagD).substr(1));
  if (pos_start == std::string_view::npos) return cSubstring();
// start tag found at pos_start. Now search the end tag
  pos_start += N + 1; // start of ... between tags
  *(tagD.data() + 1) = '/';
  size_t len = sv.substr(pos_start).rfind(tagD);
  if (len == std::string_view::npos) return cSubstring();
  return cSubstring(pos_start, len);
}
template<std::size_t N> inline
cSv partInXmlTag(cSv sv, const char (&tag)[N]) {
  return substringInXmlTag(sv, tag).substr(sv);
}
template<std::size_t N, std::size_t M> inline
cToSvConcat<N> &eraseXmlTag(cToSvConcat<N> &target, const char (&tag)[M]) {
  return substringInXmlTag(target, tag).erase(target, M-1);
}
template<std::size_t M> inline
std::string &eraseXmlTag(std::string &target, const char (&tag)[M]) {
  return substringInXmlTag(target, tag).erase(target, M-1);
}
template<std::size_t N> inline
cSubstring cSubstring::substringInXmlTag(cSv sv, const char (&tag)[N]) {
  cSubstring res = ::substringInXmlTag(substr(sv), tag);
  if (res.found() ) res.m_pos_start += m_pos_start;
  return res;
}

// =========================================================
// =========================================================
// Chapter 6: containers
// convert containers to strings, and strings to containers
// =========================================================
// =========================================================

/*
 * class cSplit: iterate over parts of a string
   standard constructor:
     delimiter is ONLY between parts, and not at beginning or end of string
     a string with n delimiters splits into n+1 parts. Always. Parts can be empty
     consequence:
       an empty string (0 delimiters) results in a list with one (empty) entry
       a delimiter at the beginning of the string results in a first (empty) part
   constructor with additional parameter of type eSplitDelimBeginEnd:
     eSplitDelimBeginEnd::optional:
       an empty string results in an empty list
       a string with length 1 containing only the delimiter results in an empty list
       a string with length 2 containing only delimiters results in a list with one (empty) entry
       otherwise, if there is a delimiter at beginning and/or end of string, delete these delimiters.
       after that, continue with standard constructor.
     eSplitDelimBeginEnd::required:
       empty string (length == 0):
         -> empty list (this is not possible with optional!)
       string with length == 1:
         must contain the delimiter (otherwise, error message in syslog)
         -> empty list (this is not possible with optional!)
       string with length > 1:
         must contain the delimiter at beginning and end of string (otherwise, error message in syslog)
         -> a string with n delimiters will split into n-1 parts

  note: for strings created with cContainer use eSplitDelimBeginEnd::required

*/
enum class eSplitDelimBeginEnd { none, optional, required };
inline cSv trim_delim(cSv sv, char delim, eSplitDelimBeginEnd splitDelimBeginEnd) {
// if trunc remove delim from start and end of sv
  switch (splitDelimBeginEnd) {
    case eSplitDelimBeginEnd::none    : return sv;
    case eSplitDelimBeginEnd::optional:
      if (sv.empty() ) return sv;
      if (sv[sv.length()-1] == delim) {
// delim at end
        if (sv.length() == 1) return cSv(); // remove delim
        if (sv[0] == delim) return sv.substr(1, sv.length() - 2);  // remove delim at begin and end
        return sv.substr(0, sv.length() - 1);  // remove delim at end
      }
// no delim at end
      if (sv[0] == delim) return sv.substr(1); // remove delim at begin
      return sv;
    case eSplitDelimBeginEnd::required:
      if (sv.empty() ) return sv;
      if ((sv[0] != delim) | (sv[sv.length()-1] != delim)) {
        esyslog(PLUGIN_NAME_I18N ": ERROR trim_delim, delim missing, sv: \"%.*s\", delim: \"%c\"", (int)sv.length(), sv.data(), delim);
        return sv;
      }
      if (sv.length() == 1) return cSv();
      return sv.substr(1, sv.length() - 2);
  }
  return sv;
}
template<class TV=cSv>  // TV is the value type returned by the const_iterator
class cSplit {
  public:
    cSplit(cSv sv, char delim, const char *context = nullptr):
      m_sv(sv), m_delim(delim), m_end(sv, delim, context), m_empty(false), m_context(context) { }
    cSplit(cSv sv, char delim, eSplitDelimBeginEnd splitDelimBeginEnd, const char *context = nullptr):
      m_sv(trim_delim(sv, delim, splitDelimBeginEnd)),
      m_delim(delim),
      m_end(m_sv, delim, context),
      m_empty((sv.length() < 2) & (
              (splitDelimBeginEnd == eSplitDelimBeginEnd::required) |
              (m_sv.empty() & (splitDelimBeginEnd == eSplitDelimBeginEnd::optional))  ) ),
      m_context(context)
    { }
    cSplit(const cSplit&) = delete;
    cSplit &operator= (const cSplit &) = delete;
    class const_iterator {
        const cSv m_sv;
        const char m_delim;
        const char *m_context;
        cSv::size_type m_start;
        cSv::size_type m_end;
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TV;
        using difference_type = int;
        using pointer = const TV*;
        using reference = const TV&;

        explicit const_iterator(cSv sv = cSv(), char delim = ':', const char *context = nullptr, bool begin = false):
          m_sv(sv), m_delim(delim), m_context(context) {
          if (begin) {
            m_start = 0;
            m_end = m_sv.find2(m_delim);
          } else {
            m_start = sv.length() + 1;
            m_end = cSv::npos;
          }
        }
        const_iterator& operator++() {
          m_start = m_end+1;
          if (m_end == m_sv.length() )
            m_end = cSv::npos;
          else
            m_end = m_sv.find2(m_delim, m_start);
          return *this;
        }
        const_iterator& operator--() {
          m_end = m_start-1;
          for (m_start = m_end; m_start > 0;) {
            --m_start;
            if (m_sv[m_start] == m_delim) {
              ++m_start;
              return *this;
            }
          }
          return *this;
        }
        bool operator!=(const_iterator other) const { return m_end != other.m_end; }
        bool operator==(const_iterator other) const { return m_end == other.m_end; }
        cSv value() const {
          return m_sv.substr(m_start, m_end-m_start);
        }
        TV operator*() const { return lexical_cast<TV>(value()); }
      void getValues() {};
      template<typename T, typename... Args>
      void getValues(T &n, Args&&... args) {
        if (m_end == cSv::npos) {
          n = T();
        } else {
          n = lexical_cast<T>(value(), T(), m_context);
          (*this).operator++();
        }
        return getValues(std::forward<Args>(args)...);
      }
    };
    typedef typename cSplit<TV>::const_iterator iterator;
    template<typename... Args> size_t getValues(Args&&... args) {
      begin().getValues(std::forward<Args>(args)...);
      return size();
    }
    const_iterator begin() const { return m_empty?m_end:const_iterator(m_sv, m_delim, m_context, true); }
//    const const_iterator &end() const { return m_end; }
    const_iterator end() const { return m_end; }
    static const_iterator s_end() { return const_iterator(); }
    const_iterator find(cSv sv) {
      if (m_sv.find(sv) == std::string_view::npos) return m_end;
      return std::find(begin(), end(), sv);
    }
    bool empty() const { return m_empty; }
    size_t size() const {
      if (m_empty) return 0;
      return std::count(m_sv.begin(), m_sv.end(), m_delim)+1;
    }
  private:
    const cSv m_sv;
    const char m_delim;
    const const_iterator m_end;
    const bool m_empty;
    const char *m_context;
};

/*
 * class cRange: create a "range" class from begin & end iterator
*/
template<class I> class cRange {
  public:
    cRange(I begin, I end): m_begin(begin), m_end(end) {}
    void set_begin(I begin) { m_begin = begin; }
    void set_end(I end) { m_end = end; }
    using const_iterator = I;
    using iterator = I;
    I begin() { return m_begin; }
    I end()   { return m_end; }
  private:
    I m_begin;
    I m_end;
};

/*
 * class cUnion: iterate over serveral containers, as if it was one.
 * value_type of first container will be used.
*/
template<class T_V, class...U> class cUnion {};
template<class T_V> class cUnion<T_V> {
  public:
    typedef T_V* iterator;
    iterator begin() { return nullptr; }
    iterator end()   { return nullptr; }
};
template<class T_V, class T, class...U>
class cUnion<T_V, T, U...> {
  public:
    cUnion(T& c1, U&...c2): m_sf1(c1), m_sf2(c2...) { }
      using T_I = typename T::iterator;
      using T_I2 = typename cUnion<T_V, U...>::iterator;
    class iterator {
        T_I m_it1;
        T_I m_it1_end;
        T_I2 m_it2;
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T_V;
        using difference_type = int;
        using pointer = const T_V*;
        using reference = T_V&;

        explicit iterator(T_I it1, T_I it1_end, T_I2 it2):
          m_it1(it1), m_it1_end(it1_end), m_it2(it2) {}
        iterator& operator++() {
          if (m_it1 != m_it1_end) ++m_it1;
          else ++m_it2;
          return *this;
        }
        bool operator!=(iterator other) const { return m_it1 != other.m_it1  || m_it2 != other.m_it2; }
        bool operator==(iterator other) const { return m_it1 == other.m_it1  && m_it2 == other.m_it2; }
        T_V operator*() const {
          if (m_it1 != m_it1_end) return *m_it1;
          else return *m_it2;
        }
      };
      iterator begin() { return iterator(m_sf1.begin(), m_sf1.end(), m_sf2.begin() ); }
      iterator end()   { return iterator(m_sf1.end(),   m_sf1.end(), m_sf2.end()   ); }
  private:
    T& m_sf1;
    cUnion<T_V, U...> m_sf2;
};
template<class V1, class ...V> cUnion(V1& c1, V&...c) -> cUnion<typename std::iterator_traits<typename V1::const_iterator>::value_type, V1, V...>;

/*
 * class cContainer: combine strings in one string
 * adding a string which is already in the container will be ignored
*/
class cContainer {
  public:
    cContainer(char delim = '|'): m_delim(delim) { }
// start with delimiter. This allows 'empty' items
    cContainer(const cContainer&) = delete;
    cContainer &operator= (const cContainer &) = delete;
    bool find(cSv sv) {
      char ns[sv.length() + 2];
      ns[0] = m_delim;
      ns[sv.length() + 1] = m_delim;
      memcpy(ns + 1, sv.data(), sv.length());
      size_t f = m_buffer.find(ns, 0, sv.length()+2);
      return f != std::string_view::npos;
    }
    bool insert(cSv sv) {
// true, if already in buffer (will not insert again ...)
// else: false
      if (m_buffer.empty() ) {
        m_buffer.reserve(300);
        m_buffer.append(1, m_delim);
      } else if (find(sv)) return true;
      m_buffer.append(sv);
      m_buffer.append(1, m_delim);
      return false;
    }
    std::string moveBuffer() { return std::move(m_buffer); }
    const std::string &getBufferRef() { return m_buffer; }
  private:
    char m_delim;
    std::string m_buffer;
};
/*
 * class cSortedVector:
 *   - unique elements only
 *   - inser with O(N)
 *   - search with O(log(N))
 * see https://lafstern.org/matt/col1.pdf
*/
template <class T, class Compare = std::less<T> >
class cSortedVector {
  public:
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::reverse_iterator reverse_iterator;
    typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;
    typedef typename std::vector<T>::size_type size_type;

    cSortedVector(const Compare& c = Compare()): m_v(), m_cmp(c) {}
    template <class InputIterator>
    cSortedVector(InputIterator first, InputIterator last, const Compare& c = Compare()):
      m_v(first, last), m_cmp(c) {
        std::sort(begin(), end(), m_cmp);
        m_v.erase(std::unique( m_v.begin(), m_v.end() ), m_v.end() );
// for std::unique: The removing operation is stable: the relative order of the elements not to be removed stays the same.
      }
    cSortedVector(std::initializer_list<T> init, const Compare& c = Compare()):
      m_v(init), m_cmp(c) {
        std::sort(begin(), end(), m_cmp);
        m_v.erase(std::unique( m_v.begin(), m_v.end() ), m_v.end() );
    }

    iterator begin() { return m_v.begin(); }
    iterator end() { return m_v.end(); }
    const_iterator begin() const { return m_v.cbegin(); }
    const_iterator end() const { return m_v.cend(); }
    const_iterator cbegin() const { return m_v.cbegin(); }
    const_iterator cend() const { return m_v.cend(); }
    reverse_iterator rbegin() { return m_v.rbegin(); }
    reverse_iterator rend() { return m_v.rend(); }
    const_reverse_iterator rbegin() const { return m_v.crbegin(); }
    const_reverse_iterator rend() const { return m_v.crend(); }
    const_reverse_iterator crbegin() const { return m_v.crbegin(); }
    const_reverse_iterator crend() const { return m_v.crend(); }

    bool empty() const { return m_v.empty(); }
    size_type size() const { return m_v.size(); }
    void clear() { m_v.clear(); }
    void reserve(size_type new_cap) { m_v.reserve(new_cap); }

    iterator insert(const T& t) {
      iterator i = std::lower_bound(begin(), end(), t, m_cmp);
      if (i == end() || m_cmp(t, *i)) m_v.insert(i, t);
      return i;
    }
    template<class K> iterator find(const K& x) {
      iterator i = std::lower_bound(begin(), end(), x, m_cmp);
      return i == end() || m_cmp(x, *i) ? end() : i;
    }
    template<class K> const_iterator find(const K& x) const {
      const_iterator i = std::lower_bound(begin(), end(), x, m_cmp);
      return i == end() || m_cmp(x, *i) ? end() : i;
    }
  private:
    std::vector<T> m_v;
    Compare m_cmp;
};

/*
   ================ regex ==============================================
   flags:
  ECMAScript Use the Modified ECMAScript regular expression grammar.
  icase      Character matching should be performed without regard to case.
  nosubs     When performing matches, all marked sub-expressions (expr) are treated as non-marking sub-expressions (?:expr). No matches are stored in the supplied std::regex_match structure and mark_count() is zero.
  optimize 	 Instructs the regular expression engine to make matching faster, with the potential cost of making construction slower. For example, this might mean converting a non-deterministic FSA to a deterministic FSA.
  collate 	 Character ranges of the form "[a-b]" will be locale sensitive.
  multiline (C++17) Specifies that ^ shall match the beginning of a line and $ shall match the end of a line, if the ECMAScript engine is selected.


*/
inline std::regex getRegex(cSv sv, const std::locale &locale, std::regex::flag_type flags = std::regex_constants::icase | std::regex_constants::collate) {
  try {
    std::regex result;
    result.imbue(locale);
    result.assign(sv.data(), sv.length(), flags);
    return result;
  } catch (const std::regex_error& e)
  {
    esyslog(PLUGIN_NAME_I18N "%s", cToSvConcat(": ERROR ", e.what(), " in regex ", sv).c_str() );
    return std::regex();
  }
}

// =========================================================
// Utility to measure times (performance) ****************
// =========================================================
class cMeasureTime {
  public:
    void start() { begin = std::chrono::high_resolution_clock::now(); }
    void stop()  {
      std::chrono::duration<double> timeNeeded = std::chrono::high_resolution_clock::now() - begin;
      maxT = std::max(maxT, timeNeeded);
      sumT += timeNeeded;
      ++numCalls;
    }
    void reset() {
      sumT = std::chrono::duration<double>(0);
      maxT = std::chrono::duration<double>(0);
      numCalls = 0;
    }
    void add(const cMeasureTime &other) {
      maxT = std::max(maxT, other.maxT);
      sumT += other.sumT;
      numCalls += other.numCalls;
    }
    void print(const char *context) const {
      if (numCalls == 0) return;
      if (!context) context = "cMeasureTime";
      dsyslog("%s num = %5i, time = %9.5f, average %f, max = %f", context, numCalls, sumT.count(), sumT.count()/numCalls, maxT.count());
    }
    int getNumCalls() const { return numCalls; }

  private:
    int numCalls = 0;
    std::chrono::duration<double> sumT = std::chrono::duration<double>(0.);
    std::chrono::duration<double> maxT = std::chrono::duration<double>(0.);
    std::chrono::time_point<std::chrono::high_resolution_clock> begin;
};

#endif // __STRINGHELPERS_H
