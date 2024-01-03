/*
 * version 0.9.2
 * general stringhelper functions
 * Note: currently, most up to date Version is in live!
 *
 * only depends on g++ -std=c++17 std:: standard headers and on esyslog (from VDR)
 * an on vdr channels :( .
 *
 * no other dependencies, so it can be easily included in any other header
 *
 *
*/
#ifndef __STRINGHELPERS_H
#define __STRINGHELPERS_H

#include <cstdarg>
#include <string>
#include <string_view>
#include <string.h>
#include <vector>
#include <set>
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
// challange:
//   method with importing parameter std::string_view called with const char * = nullptr
//   undefined behavior, as std::string_view(nullptr) is undefined. In later c++ standard, it is even an abort
// solution:
//   a) be very carefull, check const char * for nullptr before calling a method with std::string_view as import parameter
// or:
//   b) replace all std::string_view with cSv
//      very small performance impact if such a method if called with cSv
//      this will convert nullptr to empty cSv if called with const char *

// 2nd advantage of cSv: substr(pos) if pos > length: no dump, just an empty cSv as result

inline wint_t Utf8ToUtf32(const char *&p, int len);
class utf8_iterator;
class cSv: public std::string_view {
  friend class utf8_iterator;
  public:
    cSv(): std::string_view() {}
    template<std::size_t N> cSv(const char (&s)[N]): std::string_view(s, N-1) {
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
    cSv(const char *s, size_t l): std::string_view(s, l) {}
    cSv(const unsigned char *s, size_t l): std::string_view(reinterpret_cast<const char *>(s), l) {}
    cSv(std::string_view sv): std::string_view(sv) {}
    cSv(const std::string &s): std::string_view(s) {}
    cSv substr(size_t pos) const { return (length() > pos)?cSv(data() + pos, length() - pos):cSv(); }
    cSv substr(size_t pos, size_t count) const { return (length() > pos)?cSv(data() + pos, std::min(length() - pos, count) ):cSv(); }
  private:
    static std::string_view charPointerToStringView(const char *s) {
      return s?std::string_view(s, strlen(s)):std::string_view();
    }
// =================================================
// *********   utf8  *****************
// =================================================
    int utf8CodepointIsValid(size_t pos) const {
// In case of invalid UTF8, return 0
// In case of invalid input, return -1 (pos  >= sc.length())
// otherwise, return number of characters for this UTF codepoint
// note: pos + number of characters <= sc.length(), this is checked (otherwise 0 is returned)

      if (pos >= length() ) return -1;
      static const uint8_t LEN[] = {2,2,2,2,3,3,4,0};

      int len = (((*this)[pos] & 0xC0) == 0xC0) * LEN[((*this)[pos] >> 3) & 7] + (((*this)[pos] | 0x7F) == 0x7F);
      if (len == 1) return 1;
      if (len + pos > length()) return 0;
      for (size_t k= pos + 1; k < pos + len; k++) if (((*this)[k] & 0xC0) != 0x80) len = 0;
      return len;
    }
    size_t utf8ParseBackwards(size_t pos) const {
// pos <= s.length()! this is not checked
// return position of character before pos
// see also https://stackoverflow.com/questions/22257486/iterate-backwards-through-a-utf8-multibyte-string
      for (size_t i = pos; i > 0; ) {
        --i;
        if (((*this)[i] & 0xC0) != 0x80) return i;
// (s[i] & 0xC0) == 0x80 is true if bit 6 is clear and bit 7 is set
      }
      return 0;
    }
  public:
    utf8_iterator utf8_begin() const;
    utf8_iterator utf8_end() const;
};

// iterator for utf8
class utf8_iterator {
    const cSv m_sv;
    size_t m_pos;
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
//       i.e. to an ascii (bit 7 not set) or and utf8 start byte (bit 6&7 set)
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
      const char *p = m_sv.data() + m_pos;
      return Utf8ToUtf32(p, l);
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


// =========================================================
// cStr: similar to cSv, but support c_str()
// =========================================================

class cStr {
  public:
    cStr() {}
    cStr(const char *s) { if (s) m_s = s; }
    cStr(const unsigned char *s) { if (s) m_s = reinterpret_cast<const char *>(s); }
    cStr(const std::string &s): m_s(s.c_str()) {}
    operator const char*() const { return m_s; }
    const char *c_str() const { return m_s; }
    const char *data() const { return m_s; }
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

inline int AppendUtfCodepoint(char *&target, wint_t codepoint) {
  if (codepoint <= 0x7F) {
    if (target) {
      *(target++) = (char) (codepoint);
      *target = 0;
    }
    return 1;
  }
  if (codepoint <= 0x07FF) {
    if (target) {
      *(target++) =( (char) (0xC0 | (codepoint >> 6 ) ) );
      *(target++) =( (char) (0x80 | (codepoint & 0x3F)) );
      *target = 0;
    }
    return 2;
  }
  if (codepoint <= 0xFFFF) {
    if (target) {
      *(target++) =( (char) (0xE0 | ( codepoint >> 12)) );
      *(target++) =( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
      *(target++) =( (char) (0x80 | ( codepoint & 0x3F)) );
      *target = 0;
    }
    return 3;
  }
    if (target) {
      *(target++) =( (char) (0xF0 | ((codepoint >> 18) & 0x07)) );
      *(target++) =( (char) (0x80 | ((codepoint >> 12) & 0x3F)) );
      *(target++) =( (char) (0x80 | ((codepoint >>  6) & 0x3F)) );
      *(target++) =( (char) (0x80 | ( codepoint & 0x3F)) );
      *target = 0;
    }
  return 4;
}

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
// In case of invalid UTF8, return 0
// otherwise, return number of characters for this UTF codepoint
  static const uint8_t LEN[] = {2,2,2,2,3,3,4,0};

  int len = ((*p & 0xC0) == 0xC0) * LEN[(*p >> 3) & 7] + ((*p | 0x7F) == 0x7F);
  for (int k=1; k < len; k++) if ((p[k] & 0xC0) != 0x80) len = 0;
  return len;
}
inline wint_t Utf8ToUtf32(const char *&p, int len) {
// assumes, that uft8 validity checks have already been done. len must be provided. call utf8CodepointIsValid first
// change p to position of next codepoint (p = p + len)
  static const uint8_t FF_MSK[] = {0xFF >>0, 0xFF >>0, 0xFF >>3, 0xFF >>4, 0xFF >>5, 0xFF >>0, 0xFF >>0, 0xFF >>0};
  wint_t val = *p & FF_MSK[len];
  const char *q = p + len;
  for (p++; p < q; p++) val = (val << 6) | (*p & 0x3F);
  return val;
}

inline wint_t getUtfCodepoint(const char *p) {
// get next codepoint
// 0 is returned at end of string
  if(!p || !*p) return 0;
  int l = utf8CodepointIsValid(p);
  if( l == 0 ) return '?';
  const char *s = p;
  return Utf8ToUtf32(s, l);
}

inline wint_t getNextUtfCodepoint(const char *&p){
// get next codepoint, and increment p
// 0 is returned at end of string, and p will point to the end of the string (0)
  if(!p || !*p) return 0;
  int l = utf8CodepointIsValid(p);
  if( l == 0 ) { p++; return '?'; }
  return Utf8ToUtf32(p, l);
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
  for (size_t i = sv.length() - 1; i >= 0; --i) {
    i = sv.find_last_not_of(' ', i);
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

template<class T> inline T parse_unsigned_internal(cSv sv) {
  T val = 0;
  for (size_t start = 0; start < sv.length() && std::isdigit(sv[start]); ++start) val = val*10 + (sv[start]-'0');
  return val;
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
// split sting at delimiter in two parts
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
// Otherwise, return part of str after first occurence of delim
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
//   cToSv classes, with buffer containing text reprexentation of data
// =========================================================
// =========================================================

// =========================================================
// integer and hext
// =========================================================

namespace stringhelpers_internal {

  static const char cDigitsLut[200] = {
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

// max uint16_t 65535
inline void itoa4w0(char *b, uint16_t i) {
  memcpy(b+2, cDigitsLut + ((i%100) << 1), 2);
  memcpy(b  , cDigitsLut + ((i/100) << 1), 2);
}
inline void itoa8w0(char *b, uint32_t i) {
  itoa4w0(b+4, i%10000);
  itoa4w0(b  , i/10000);
}
inline char *itoa2(char *b, uint16_t i) {
  if (i >= 10) {
    memcpy(b, cDigitsLut + (i << 1), 2);
    return b+2;
  } else {
    *b = i + '0';
    return b+1;
  }
}
// max uint16_t 65535
inline char *itoa4(char *b, uint16_t i) {
  if (i < 100) return itoa2(b, i);
  b = itoa2(b, i/100);
  memcpy(b, cDigitsLut + ((i%100) << 1), 2);
  return b+2;
}
// max uint32_t 4294967295  (10 digits)
inline char *itoa8(char *b, uint32_t i) {
  if (i < 10000) return itoa4(b, i);
  b = itoa4(b, i/10000);
  itoa4w0(b, i%10000);
  return b+4;
}
template<typename T> inline char *itoa16(char *b, T i) {
  if (i < 100000000) return itoa8(b, static_cast<uint32_t>(i));
  b = itoa8(b, static_cast<uint32_t>(i/100000000));
  itoa8w0(b, static_cast<uint32_t>(i%100000000));
  return b+8;
}

// max uint32_t 4294967295  (10 digits)
template<typename T, std::enable_if_t<sizeof(T) <= 4, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
inline char *itoa(char *b, T i) {
  if (i < 100000000) return itoa8(b, i);
  b = itoa2(b, i/100000000);  // 4294967295/100000000 = 42;
  itoa8w0(b, i%100000000);
  return b+8;
}
// max uint64_t 18446744073709551615  (20 digits)
template<typename T, std::enable_if_t<sizeof(T) >= 5, bool> = true, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
char *itoa(char *b, T i) {
  if (i < 10000000000000000) return itoa16(b, i);
  b = itoa4(b, i/10000000000000000);   // 18446744073709551615/10000000000000000 = 1844
  T r = i%10000000000000000;
  itoa8w0(b + 8, static_cast<uint32_t>(r%100000000));
  itoa8w0(b    , static_cast<uint32_t>(r/100000000));
  return b+16;
}
template<typename T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
char *itoa(char *b, T i) {
  typedef std::conditional_t<sizeof(T) >= 5, uint64_t, uint32_t> TU;
  TU u = static_cast<TU>(i);
  if (i < 0) {
      *b++ = '-';
      u = ~u + 1;
  }
  return itoa(b, u);
}

template<typename T, std::enable_if_t<sizeof(T) >= 5, bool> = true>
  inline int numCharsUg0(T i) {
// note: i must be >= 0!!!!
      int numChars = 1;
      if (i >= 10000000000000000) { i /= 10000000000000000; numChars += 16; }
      if (i >= 100000000) { i /= 100000000; numChars += 8; }
      if (i >= 10000) { i /= 10000; numChars += 4; }
      if (i >= 1000) return numChars+3;
      if (i >= 100 ) return numChars+2;
      if (i >= 10  ) return numChars+1;
      return numChars;
  }
template<typename T, std::enable_if_t<sizeof(T) <= 4, bool> = true>
  inline int numCharsUg0(T i) {
// note: i must be >= 0!!!!
      int numChars = 1;
      if (i >= 100000000) { i /= 100000000; numChars += 8; }
      if (i >= 10000) { i /= 10000; numChars += 4; }
      if (i >= 1000) return numChars+3;
      if (i >= 100 ) return numChars+2;
      if (i >= 10  ) return numChars+1;
      return numChars;
  }
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
  inline int numChars(T i) {
    return numCharsUg0(i);
  }
template<typename T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
  inline int numChars(T i) {
    if (i >= 0) return numCharsUg0(i);
    return numCharsUg0(-i)+1;
  }
  template<class T> inline char *addCharsUg0be(char *be, T i) {
// i > 0 must be ensured before calling!
// make sure to have a large enough buffer size (20 + zero terminator if required)
// be is buffer end. Last character is written to be-1
// no zero terminator is written! You can make buffer large enough and set *be=0 before calling
// position of first char is returned
// length is be - returned value
    const char *lt = cDigitsLut;
    while (i >= 10) {
      be -= 2;
      memcpy(be, lt + ((i%100) << 1), 2);
/* memcpy is about 4% faster
       int r2 = (i%100) << 1;
       *(--be) = lt[r2+1];
       *(--be) = lt[r2];
*/
      i /= 100;
    }
    if (i>0) *(--be) = '0' + (i%10);
    return be;
  }
  template<class T> inline char *addCharsIbe(char *be, T i) {
// i can be any integer like type (signed, unsigned, ...)
// only for internal use. Please use class cToSvInt instead
//
// make sure to have a large enough buffer size (20 + zero terminator if required)
// be is buffer end. Last character is written to be-1
// no zero terminator is written! You can make buffer large enough and set *be=0 before calling
// position of first char is returned
// length is be - returned value
// Example:
//  char buffer_i[21];
//  buffer_i[20] = 0;
//  std::cout << "\"" << stringhelpers_internal::addCharsIbe(buffer_i+20, 5) << "\"\n";
// Example 2:
//  char buffer2_i[20];
//  char *result = stringhelpers_internal::addCharsIbe(buffer2_i+20, 6);
//  std::cout << "\"" << cSv(result, buffer2_i + 20  - result)  << "\"\n";

    if (i > 0) return addCharsUg0be(be, i);
    if (i == 0) {
      *(--be) = '0';
      return be;
    }
    be = addCharsUg0be(be, -i);
    *(--be) = '-';
    return be;
  }

template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
  inline T addCharsHex(char *buffer, size_t num_chars, T value) {
// sizeof(buffer) must be >= num_chars. This is not checked !!!
// value must be >= 0. This is not checked !!!
// value is written with num_chars chars
//   if value is too small -> left values filled with 0
//   if value is too high  -> the highest numbers are not written. This is not checked!
//           but, you can check: if the returnde value is != 0, some chars are not written
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
    virtual ~cToSv() {}
    virtual operator cSv() const = 0;
};
inline std::ostream& operator<<(std::ostream& os, cToSv const& sv )
{
  return os << cSv(sv);
}

template<std::size_t N>
class cToSvHex: public cToSv {
  public:
template<class T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    cToSvHex(T value) {
      stringhelpers_internal::addCharsHex(m_buffer, N, value);
    }
    operator cSv() const { return cSv(m_buffer, N); }
  protected:
    cToSvHex() { }
    char m_buffer[N];
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
        if (errno_l != ENOENT) esyslog("cOpen::checkError, ERROR: open fails, errno %d, filename %s\n", errno_l, pathname);
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
      cOpen fd(filename, O_RDONLY);
      if (!fd.exists()) return;
      struct stat buffer;
      if (fstat(fd, &buffer) != 0) {
        if (errno != ENOENT) esyslog("cToSvFile::load, ERROR: in fstat, errno %d, filename %s\n", errno, filename);
        return;
      }
// file exists, length buffer.st_size
      m_exists = true;
      if (buffer.st_size == 0) return; // empty file
      size_t length = buffer.st_size;
      if (max_length != 0 && length > max_length) length = max_length;
      m_s = (char *) malloc((length + 1) * sizeof(char));  // add one. So we can add the 0 string terminator
      if (!m_s) {
        esyslog("cToSvFile::load, ERROR out of memory, filename = %s, requested size = %zu\n", filename, length + 1);
        return;
      }
      size_t num_read = 0;
      ssize_t num_read1 = 1;
      for (; num_read1 > 0 && num_read < length; num_read += num_read1) {
        num_read1 = read(fd, m_s + num_read, length - num_read);
        if (num_read1 == -1) {
          esyslog("cToSvFile::load, ERROR: read failed, errno %d, filename %s\n", errno, filename);
          m_s[0] = 0;
          return;
        }
      }
      m_result = cSv(m_s, num_read);
      m_s[num_read] = 0;  // so data returns a 0 terminated string
      if (num_read != length) {
        esyslog("cToSvFile::load, ERROR: num_read = %zu, length = %zu, filename %s\n", num_read, length, filename);
      }
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
        esyslog("cToSvFile::load, ERROR: read fails, errno %d, filename %s\n", errno, filename);
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
template<std::size_t N = 255>
class cToSvConcat: public cToSv {
  public:
    cToSvConcat() {}
    template<typename... Args> cToSvConcat(Args&&... args) {
      concat(std::forward<Args>(args)...);
    }
    template<typename T, typename U, typename... Args>
    cToSvConcat &concat(T &&n, U &&u, Args&&... args) {
      concat(n);
      return concat(std::forward<U>(u), std::forward<Args>(args)...);
    }
// ========================
// overloads for concat
    cToSvConcat &concat(char ch) {
      if (m_pos_for_append == m_be_data) ensure_free(1);
      *(m_pos_for_append++) = ch;
      return *this;
    }
    cToSvConcat &concat(cSv sv) { return append(sv.data(), sv.length()); }

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvConcat &concat(T i) {
      if (m_pos_for_append + 20 > m_be_data) ensure_free(20);
      m_pos_for_append = stringhelpers_internal::itoa(m_pos_for_append, i);
      return *this;
    }
template<typename T> cToSvConcat &operator<<(T sv) { return concat(sv); }

// ========================
// overloads for append. Should be compatible to std::string.append(...)
// ========================
    cToSvConcat &append(cSv sv) { return append(sv.data(), sv.length()); }
    cToSvConcat &append(const char *s, size_t len) {
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
// appendInt   append integer (with some format options)
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvConcat &appendInt(T i, size_t desired_width, char fill_char = '0') {
      size_t len = stringhelpers_internal::numChars(i);
      if (desired_width <= len) return concat(i);
      if (m_pos_for_append + desired_width > m_be_data) ensure_free(desired_width);
      if (i < 0 && fill_char == '0') {
        *m_pos_for_append++ = '-';
        i = -i;
      }
      memset(m_pos_for_append, fill_char, desired_width-len);
      m_pos_for_append += desired_width-len;
      m_pos_for_append = stringhelpers_internal::itoa(m_pos_for_append, i);
      return *this;
    }
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    cToSvConcat &appendHex(T value, int width) {
      if (m_pos_for_append + width > m_be_data) ensure_free(width);
      stringhelpers_internal::addCharsHex(m_pos_for_append, width, value);
      m_pos_for_append += width;
      return *this;
    }
// =======================
// append_utf8 append utf8 codepoint
    cToSvConcat &append_utf8(wint_t codepoint) {
      if (m_pos_for_append + 4 > m_be_data) ensure_free(4);
      AppendUtfCodepoint(m_pos_for_append, codepoint);
      return *this;
    }
// =======================
// appendToLower
    void appendToLower(cSv sv, const std::locale &loc) {
      for (auto it = sv.utf8_begin(); it != sv.utf8_end(); ++it) {
        append_utf8(std::tolower<wchar_t>(*it, loc));
      }
    }
// =======================
// appendFormated append formated
// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with gcc 13.1
    template<typename... Args> cToSvConcat &appendFormated(const char *fmt, Args&&... args) {
      int needed = snprintf(m_pos_for_append, m_be_data - m_pos_for_append, fmt, std::forward<Args>(args)...);
      if (needed < 0) {
        esyslog("live: ERROR, cToScConcat::appendFormated needed = %d, fmt = %s", needed, fmt);
        return *this; // error in snprintf
      }
      if (needed < m_be_data - m_pos_for_append) {
        m_pos_for_append += needed;
        return *this;
      }
      ensure_free(needed + 1);
      needed = sprintf(m_pos_for_append, fmt, std::forward<Args>(args)...);
      if (needed < 0) {
        esyslog("live: ERROR, cToScConcat::appendFormated needed (2) = %d, fmt = %s", needed, fmt);
        return *this; // error in sprintf
      }
      m_pos_for_append += needed;
      return *this;
    }
// =======================
// appendDateTime: append date/time formated with strftime
    cToSvConcat &appendDateTime(const char *fmt, const std::tm *tp) {
      size_t needed = std::strftime(m_pos_for_append, m_be_data - m_pos_for_append, fmt, tp);
      if (needed == 0) {
        ensure_free(1024);
        needed = std::strftime(m_pos_for_append, m_be_data - m_pos_for_append, fmt, tp);
        if (needed == 0) {
          esyslog("live: ERROR, cToScConcat::appendDateTime needed = 0, fmt = %s", fmt);
          return *this; // we did not expect to need more than 1024 chars for the formated time ...
        }
      }
      m_pos_for_append += needed;
      return *this;
    }
    cToSvConcat &appendDateTime(const char *fmt, time_t time) {
      if (!time) return *this;
      struct std::tm tm_r;
      if (localtime_r( &time, &tm_r ) == 0 ) {
        esyslog("live: ERROR, cToScConcat::appendDateTime localtime_r = 0, fmt = %s, time = %lld", fmt, (long long)time);
        return *this; // we did not expect to need more than 1024 chars for the formated time ...
        }
      return appendDateTime(fmt, &tm_r);
    }
// =======================
// #include <vdr/channels.h>

// =========================================================
// some performance improvemnt, to get string presentation for channel
// you can also use channelID.ToString()
// in struct tChannelID {  (in vdr):
//   static tChannelID FromString(const char *s);
//   cString ToString(void) const;
// =========================================================
// append tChannelID
    cToSvConcat &concat(const tChannelID &channelID) {
      int st_Mask = 0xFF000000;
      concat((char) ((channelID.Source() & st_Mask) >> 24));
      if (int n = cSource::Position(channelID.Source())) {
         appendFormated("%u.%u", abs(n) / 10, abs(n) % 10); // can't simply use "%g" here since the silly 'locale' messes up the decimal point
         concat( (n < 0) ? 'W' : 'E');
      }
      appendFormated(channelID.Rid() ? "-%d-%d-%d-%d" : "-%d-%d-%d",
          channelID.Nid(), channelID.Tid(), channelID.Sid(), channelID.Rid() );
      return *this;
    }
// ========================
// get data
    operator cSv() const { return cSv(m_buffer, m_pos_for_append-m_buffer); }
    char *data() { *m_pos_for_append = 0; return m_buffer; }
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
    void reserve(size_t r) const { m_reserve = r; }
    ~cToSvConcat() {
      if (m_buffer_allocated) free (m_buffer_allocated);
    }
  private:
    void ensure_free(size_t l) {
// make sure that l bytes can we written at m_pos_for_append
      if (m_pos_for_append + l <= m_be_data) return;
      size_t current_length = m_pos_for_append - m_buffer;
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
    char *m_pos_for_append = m_buffer;
    char *m_be_data = m_buffer + sizeof(m_buffer_static) - 1; // [m_buffer, m_be_data) is available for data.
// It must be possible to write the 0 terminator to m_be_data: *m_be_data = 0.
// m_pos_for_append <= m_be_data: must be always ensured.
//   m_be_data - m_pos_for_append: Number of bytes available for write
    mutable size_t m_reserve = 1024;
};

class cToSvInt: public cToSvConcat<20> {
  public:
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvInt (T i): cToSvConcat(i) { }
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cToSvInt (T i, size_t desired_width, char fill_char = '0') {
      appendInt(i, desired_width, fill_char);
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
// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with gcc 13.1
    template<typename... Args> cToSvFormated(const char *fmt, Args&&... args) {
      this->appendFormated(fmt, std::forward<Args>(args)...);
    }
};
class cToSvDateTime: public cToSvConcat<255> {
  public:
    cToSvDateTime(const char *fmt, time_t time) {
      this->appendDateTime(fmt, time);
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
  str.append(buf, stringhelpers_internal::itoa(buf, i) - buf);
}
template<std::size_t N, typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
inline void stringAppend(cToSvConcat<N> &s, T i) {
  s.concat(i);
}

template<std::size_t N, typename... Args>
inline void stringAppendFormated(cToSvConcat<N> &s, const char *fmt, Args&&... args) {
  s.appendFormated(fmt, std::forward<Args>(args)...);
}

// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with gcc 13.1
template<typename... Args>
void stringAppendFormated(std::string &str, const char *fmt, Args&&... args) {
  size_t size = 1024;
  char buf[size];
  int needed = snprintf (buf, size, fmt, std::forward<Args>(args)...);
  if (needed < 0) {
    esyslog("live: ERROR, stringAppendFormated, needed = %d", needed);
    return; // error in snprintf
  }
  if ((size_t)needed < size) {
    str.append(buf);
  } else {
    char buf2[needed + 1];
    needed = sprintf (buf2, fmt, args...);
    if (needed < 0) {
      esyslog("live: ERROR, stringAppendFormated, needed (2) = %d", needed);
      return; // error in snprintf
    }
    str.append(buf2);
  }
}
/*
 * slower than stringAppendFormated
 * also, we wast memory as needed_guess is too large
template<typename... Args>
void stringAppendFormated_2(std::string &str, const char *fmt, Args&&... args) {
  size_t needed_guess =  80;
  size_t old_len = str.length();
  str.append(needed_guess, 0);
  int needed = snprintf (str.data() + old_len, needed_guess+1, fmt, std::forward<Args>(args)...);
  if (needed < 0) {
    esyslog("live: ERROR, stringAppendFormated, needed = %d, fmt = %s", needed, fmt);
    return; // error in snprintf
  }
  if ( (size_t)needed <= needed_guess) { str.erase(old_len + needed); return; }
  str.append(needed - needed_guess, '|');
  sprintf (str.data() + old_len, fmt, std::forward<Args>(args)...);
}
 * short, and works fine with str.data()
 * but, too slow :) . Usage of buf is twice as fast ...
template<typename... Args>
void stringAppendFormated_slow(std::string &str, const char *fmt, Args&&... args) {
  int needed = snprintf (nullptr, 0, fmt, std::forward<Args>(args)...);
  if (needed < 0) {
    esyslog("live: ERROR, stringAppendFormated, needed = %d, fmt = %s", needed, fmt);
    return; // error in snprintf
  }
  size_t old_len = str.length();
  str.append(needed, '|');
  sprintf (str.data() + old_len, fmt, std::forward<Args>(args)...);
}
*/

// =========================================================
// =========== stringAppend ==  for many data types
// =========================================================

// strings
inline void stringAppend(std::string &str, const char *s) { if(s) str.append(s); }
inline void stringAppend(std::string &str, const std::string &s) { str.append(s); }
inline void stringAppend(std::string &str, std::string_view s) { str.append(s); }

inline void stringAppend(std::string &str, const tChannelID &channelID) {
  str.append(cToSvConcat(channelID));
}
template<typename T, typename U, typename... Args>
void stringAppend(std::string &str, const T &n, const U &u, Args&&... args) {
  stringAppend(str, n);
  stringAppend(str, u, std::forward<Args>(args)...);
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
// =========== concatenate =================================
// =========================================================
// deprecated. Use cToSvConcat

template<typename... Args>
inline std::string concatenate(Args&&... args) {
  std::string result;
  result.reserve(200);
  stringAppend(result, std::forward<Args>(args)...);
  return result;
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
// parse string_view for xml
// =========================================================

template<std::size_t N> cSv partInXmlTag(cSv sv, const char (&tag)[N], bool *exists = nullptr) {
// very simple XML parser
// if sv contains <tag>...</tag>, ... is returned (part between the outermost XML tags is returned).
// otherwise, cSv() is returned. This is also returned if the tags are there, but there is nothing between the tags ...
// there is no error checking, like <tag> is more often in sv than </tag>, ...
  if (exists) *exists = false;
// N == strlen(tag) + 1. It includes the 0 terminator ...
// strlen(startTag) = N+1; strlen(endTag) = N+2. Sums to 2N+3
  if (N < 1 || sv.length() < 2*N+3) return cSv();
// create <tag>
  cToSvConcat<N+2> tagD("<<", tag, ">");
  size_t pos_start = sv.find(cSv(tagD).substr(1));
  if (pos_start == std::string_view::npos) return cSv();
// start tag found at pos_start. Now search the end tag
  pos_start += N + 1; // start of ... between tags
  *(tagD.data() + 1) = '/';
  size_t len = sv.substr(pos_start).rfind(tagD);
  if (len == std::string_view::npos) return cSv();
  if (exists) *exists = true;
  return sv.substr(pos_start, len);
}

// =========================================================
// =========================================================
// Chapter 6: containers
// convert containers to strings, and strings to containers
// =========================================================
// =========================================================

class cSplit {
  public:
    cSplit(cSv sv, char delim): m_sv(sv), m_delim(delim), m_end(cSv(), m_delim) {}
// sv can start with delim (optional), and it will just be ignored
    cSplit(const cSplit&) = delete;
    cSplit &operator= (const cSplit &) = delete;
    class iterator {
        cSv m_remainingParts;
        char m_delim;
        size_t m_next_delim;
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = cSv;
        using difference_type = int;
        using pointer = const cSv*;
        using reference = cSv;

        explicit iterator(cSv r, char delim): m_delim(delim) {
          if (!r.empty() && r[0] == delim) m_remainingParts = r.substr(1);
          else m_remainingParts = r;
          m_next_delim = m_remainingParts.find(m_delim);
        }
        iterator& operator++() {
          if (m_next_delim == std::string_view::npos) {
            m_remainingParts = cSv();
          } else {
            m_remainingParts = m_remainingParts.substr(m_next_delim + 1);
            m_next_delim = m_remainingParts.find(m_delim);
          }
          return *this;
        }
        bool operator!=(iterator other) const { return m_remainingParts != other.m_remainingParts; }
        bool operator==(iterator other) const { return m_remainingParts == other.m_remainingParts; }
        cSv operator*() const {
          if (m_next_delim == std::string_view::npos) return m_remainingParts;
          else return m_remainingParts.substr(0, m_next_delim);
        }
      };
      iterator begin() { return iterator(m_sv, m_delim); }
      const iterator &end() { return m_end; }
      iterator find(cSv sv) {
        if (m_sv.find(sv) == std::string_view::npos) return m_end;
        return std::find(begin(), end(), sv);
      }
    private:
      const cSv m_sv;
      const char m_delim;
      const iterator m_end;
};

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
