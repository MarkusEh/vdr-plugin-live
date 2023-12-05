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

class cSv: public std::string_view {
  public:
    cSv(): std::string_view() {}
    cSv(const char *s): std::string_view(charPointerToStringView(s)) {}
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
};

// =========================================================
// =========================================================
// Chapter 1: UTF8 string utilities ****************
// =========================================================
// =========================================================

inline int AppendUtfCodepoint(char *&target, wint_t codepoint){
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

inline void stringAppendUtfCodepoint(std::string &target, wint_t codepoint){
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

inline int utf8CodepointIsValid(const char *p){
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
inline void stringAppendToLower(std::string &target, cSv str, const std::locale &loc) {
  const char *pos = str.data();
  const char *end = str.data() + str.length();
  while (pos < end) {
    wint_t u_char = getNextUtfCodepoint(pos);
    try {
      if (u_char) stringAppendUtfCodepoint(target, std::tolower<wchar_t>(u_char, loc));
// The standard library is guaranteed to provide the following specializations (they are required to be implemented by any locale object):
//   std::ctype<char> and std::ctype<wchar_t> . So don't call  std::tolower<wint_t>, this is not implemented !!!!
    } catch (const std::bad_cast& e) {
      esyslog("ERROR in stringToLower(%.*s,%s), error message: %s", (int) str.length(), str.data(), loc.name().c_str(), e.what() );
      return;
    }
  }
}
inline std::string stringToLower(cSv str, const std::locale &loc) {
  std::string result;
  result.reserve(str.length() + 1);
  stringAppendToLower(result, str, loc);
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
  char tagD[N + 2];
  memcpy(tagD + 2, tag, N - 1);
  tagD[N + 1] = '>';
// find <tag>
  tagD[1] = '<';
  size_t pos_start = sv.find(tagD + 1, 0, N + 1);
  if (pos_start == std::string_view::npos) return cSv();
  pos_start += N + 1; // start of ... between tags
// rfind </tag>
  tagD[0] = '<';
  tagD[1] = '/';
//  std::cout << "tagD[0] " << cSv(tagD, N + 2) << "\n";
  size_t len = sv.substr(pos_start).rfind(tagD, std::string_view::npos, N + 2);
  if (len == std::string_view::npos) return cSv();
  if (exists) *exists = true;
  return sv.substr(pos_start, len);
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
  template<class T> inline char *addCharsUg0be(char *be, T i) {
// i > 0 must be ensured before calling!
// make sure to have a large enough buffer size (20 + zero terminator if required)
// be is buffer end. Last character is written to be-1
// no zero terminator is written! You can make buffer large enough and set *be=0 before calling
// position of first char is returned
// length is be - returned value
    for (; i; i /= 10) *(--be) = '0' + (i%10);
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

  template<typename T> inline T addCharsHex_int(char *buffer, size_t num_chars, T value) {
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
  inline unsigned addCharsHex(char *buffer, size_t num_chars, unsigned value) {
    return stringhelpers_internal::addCharsHex_int(buffer, num_chars, value);
  }
  inline unsigned long addCharsHex(char *buffer, size_t num_chars, unsigned long value) {
    return stringhelpers_internal::addCharsHex_int(buffer, num_chars, value);
  }
  inline unsigned long long addCharsHex(char *buffer, size_t num_chars, unsigned long long value) {
    return stringhelpers_internal::addCharsHex_int(buffer, num_chars, value);
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

class cToSvInt: public cToSv {
  public:
// T must be an integer type, like long, unsigned, ...
    template<class T> cToSvInt(T i):
      m_result(stringhelpers_internal::addCharsIbe(m_buffer + 20, i)) {}
    operator cSv() const { return cSv(m_result, m_buffer + 20 - m_result); }
    cToSvInt &setw(size_t desired_width, char fill_char = '0') {
      char *new_m_result = m_buffer + 20 - std::min((int)desired_width, 20);
      if (m_result <= new_m_result) return *this;  // requested width alread there
      if (*m_result == '-') {
        *new_m_result = '-';
        memset(new_m_result + 1, fill_char,  m_result - new_m_result);
      } else
        memset(new_m_result, fill_char,  m_result - new_m_result);
      m_result = new_m_result;
      return *this;
    }
  private:
    char m_buffer[20]; // unsigned int 64: max. 20. (18446744073709551615) signed int64: max. 19 (+ sign)
    char *m_result;
};
template<std::size_t N>
class cToSvHex: public cToSv {
  public:
// T must be an unsigned type, like long long unsigned, ...
    template<class T> cToSvHex(T value) {
      stringhelpers_internal::addCharsHex(m_buffer, N, value);
    }
    operator cSv() const { return cSv(m_buffer, N); }
  protected:
    cToSvHex() { }
    char m_buffer[N];
};

class cToSvFile: public cToSv {
  public:
    cToSvFile(const char *filename, size_t max_length = 0) { load(filename, max_length); }
    cToSvFile(const std::string &filename, size_t max_length = 0) { load(filename.c_str(), max_length ); }
    cToSvFile(const cToSvFile&) = delete;
    cToSvFile &operator= (const cToSvFile &) = delete;
    operator cSv() const { return m_result; }
    char *data() { return m_s; } // Retunrs zero if file is empty! Is zero terminated
    const char *c_str() { return m_s?m_s:""; } // Is zero terminated
    bool exists() const { return m_exists; }
    ~cToSvFile() { std::free(m_s); }
  private:
    void load(const char *filename, size_t max_length) {
      if (!filename) return;
      int fd = open(filename, O_RDONLY);
      if (fd == -1) {
// no message for errno == ENOENT, the file just does not exist
        if (errno != ENOENT) esyslog("cToSvFile::load, ERROR: open fails, errno %d, filename %s\n", errno, filename);
        return;
      }
      struct stat buffer;
      if (fstat(fd, &buffer) != 0) {
        if (errno != ENOENT) esyslog("cToSvFile::load, ERROR: in fstat, errno %d, filename %s\n", errno, filename);
        close(fd);
        return;
      }

// file exists, length buffer.st_size
      m_exists = true;
      if (buffer.st_size == 0) { close(fd); return; } // empty file
      size_t length = buffer.st_size;
      if (max_length != 0 && length > max_length) length = max_length;
      m_s = (char *) malloc((length + 1) * sizeof(char));  // add one. So we can add the 0 string terminator
      if (!m_s) {
        esyslog("cToSvFile::load, ERROR out of memory, filename = %s, requested size = %zu\n", filename, length + 1);
        close(fd);
        return;
      }
      size_t num_read = 0;
      ssize_t num_read1 = 1;
      for (; num_read1 > 0 && num_read < length; num_read += num_read1) {
        num_read1 = read(fd, m_s + num_read, length - num_read);
        if (num_read1 == -1) {
          esyslog("cToSvFile::load, ERROR: read fails, errno %d, filename %s\n", errno, filename);
          close(fd);
          m_s[0] = 0;
          return;
        }
      }
      close(fd);
      m_result = cSv(m_s, num_read);
      m_s[num_read] = 0;  // so data returns a 0 terminated string
      if (num_read != length) {
        esyslog("cToSvFile::load, ERROR: num_read = %zu, length = %zu, filename %s\n", num_read, length, filename);
      }
    }
    bool m_exists = false;
    char *m_s = nullptr;
    cSv m_result;
};

class cToSvFormated: public cToSv {
  public:
// __attribute__ ((format (printf, 2, 3))) can not be used, but should work starting with gcc 13.1
    template<typename... Args> cToSvFormated(const char *fmt, Args&&... args) {
      int needed = snprintf (m_buffer, sizeof(m_buffer), fmt, std::forward<Args>(args)...);
      if (needed < 0) {
        esyslog("live: ERROR, cToSvFormated::cToSvFormated, needed = %d, fmt = %s", needed, fmt);
        return; // error in snprintf
      }
      if ((size_t)needed < sizeof(m_buffer)) {
        m_result = cSv(m_buffer, needed);
        return;
      }
      m_huge_buffer = (char *)std::malloc(needed + 1);
      if (m_huge_buffer == nullptr) {
        esyslog("live: ERROR, out of memory in cToSvFormated::cToSvFormated, needed = %d, fmt = %s", needed, fmt);
        return;
      }
      needed = sprintf (m_huge_buffer, fmt, args...);
      if (needed < 0) {
        esyslog("live: ERROR, cToSvFormated::cToSvFormated, needed (2) = %d, fmt = %s", needed, fmt);
        return; // error in sprintf
      }
      m_result = cSv(m_huge_buffer, needed);
    }
    ~cToSvFormated() {
      std::free(m_huge_buffer);
    }
    operator cSv() const { return m_result; }
    const char *c_str() const { return m_result.data(); }
  private:
    char m_buffer[256];
    char *m_huge_buffer = nullptr;
    cSv m_result;
};
/*
 * channel helper functions (for vdr tChannelID)
 *
*/
// #include <vdr/channels.h>

// =========================================================
// some performance improvemnt, to get string presentation for channel
// you can also use channelID.ToString()
// in struct tChannelID {  (in vdr):
//   static tChannelID FromString(const char *s);
//   cString ToString(void) const;
// =========================================================

class cToSvChannelSource: public cToSv {
  public:
    cToSvChannelSource(int Code) {
      int st_Mask = 0xFF000000;
      char *q = m_buffer;
      *q++ = (Code & st_Mask) >> 24;
      if (int n = cSource::Position(Code)) {
         q += snprintf(q, 14, "%u.%u", abs(n) / 10, abs(n) % 10); // can't simply use "%g" here since the silly 'locale' messes up the decimal point
         *q++ = (n < 0) ? 'W' : 'E';
         }
      *q = 0;
    }
    operator cSv() const { return cSv(m_buffer); }
    const char *c_str() const { return m_buffer; }
  private:
    char m_buffer[16]; // 1 + "%u.%u", sec. %u: 1 digit + 1 zero terminator
};
class cToSvChannel: public cToSvFormated {
  public:
    cToSvChannel(const tChannelID &channelID):
      cToSvFormated(channelID.Rid() ? "%s-%d-%d-%d-%d" : "%s-%d-%d-%d",
          cToSvChannelSource(channelID.Source()).c_str(),
          channelID.Nid(), channelID.Tid(), channelID.Sid(), channelID.Rid() )
      {}
};

class cToSvConcat: public cToSv {
  public:
    cToSvConcat() {}
    template<typename... Args> cToSvConcat(Args&&... args) {
      append(std::forward<Args>(args)...);
    }
    template<typename T, typename U, typename... Args>
    cToSvConcat &append(const T &n, const U &u, const Args&... args) {
      append(n);
      return append(u, args...);
    }
    cToSvConcat &append(char ch) {
      if (m_use_buffer) {
        if (m_pos_for_append  < m_be_data) {
          *(m_pos_for_append++) = ch;
          return *this;
        }
// m_buffer too small, switch to string
        m_use_buffer = false;
        m_buffer_string.reserve(2*sizeof(m_buffer) );
        m_buffer_string.append(m_buffer, m_pos_for_append-m_buffer);
      }
      m_buffer_string.append(1, ch);
      return *this;
    }
    cToSvConcat &append(cSv sv) {
      if (m_use_buffer) {
        if (m_pos_for_append + sv.length() <= m_be_data) {
          memcpy(m_pos_for_append, sv.data(), sv.length());
          m_pos_for_append += sv.length();
          return *this;
        }
// m_buffer too small, switch to string
        m_use_buffer = false;
        m_buffer_string.reserve(sizeof(m_buffer) + sv.length());
        m_buffer_string.append(m_buffer, m_pos_for_append-m_buffer);
      }
      m_buffer_string.append(sv);
      return *this;
    }
    cToSvConcat &append(int i) { return append(cToSvInt(i)); }
    cToSvConcat &append(long i) { return append(cToSvInt(i)); }
    cToSvConcat &append(long long i) { return append(cToSvInt(i)); }
    cToSvConcat &append(unsigned i) { return append(cToSvInt(i)); }
    cToSvConcat &append(unsigned long i) { return append(cToSvInt(i)); }
    cToSvConcat &append(unsigned long long i) { return append(cToSvInt(i)); }
    cToSvConcat &append(const tChannelID &channelID) { return append(cToSvChannel(channelID)); }
    template<typename T> cToSvConcat &operator<<(T sv) { return append(sv); }
    operator cSv() const { return m_use_buffer?cSv(m_buffer, m_pos_for_append-m_buffer):m_buffer_string; }
    const char *c_str() const { if (m_use_buffer) { *m_pos_for_append = 0; return m_buffer; } else return m_buffer_string.c_str(); }
  private:
    char m_buffer[256];
    char *m_pos_for_append = m_buffer;
    char *m_be_data = m_buffer + sizeof(m_buffer) - 1; // [m_buffer, m_be_data) is available for data.
    bool m_use_buffer = true;
    std::string m_buffer_string;
};


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
  short, and works fine with str.data()
  but, too slow :) . Usage of buf is twice as fast ...
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

namespace stringhelpers_internal {
// methods to append to std::strings ========================
  template<typename T>
  inline void stringAppendU(std::string &str, T i) {
// for integer types i >= 0 !!!! This is not checked !!!!!
    if (i == 0) { str.append(1, '0'); return; }
    char buf[20]; // unsigned int 64: max. 20. (18446744073709551615) signed int64: max. 19 (+ sign)
    char *bufe = buf+20;
    char *bufs = addCharsUg0be(bufe, i);
    str.append(bufs, bufe-bufs);
  }
}

// =========================================================
// =========== stringAppend ==  for many data types
// =========================================================

inline void stringAppend(std::string &str, unsigned int i) { stringhelpers_internal::stringAppendU(str, i); }
inline void stringAppend(std::string &str, unsigned long int i) { stringhelpers_internal::stringAppendU(str, i); }
inline void stringAppend(std::string &str, unsigned long long  int i) { stringhelpers_internal::stringAppendU(str, i); }

inline void stringAppend(std::string &str, int i) { str.append(cToSvInt(i)); }
inline void stringAppend(std::string &str, long int i) { str.append(cToSvInt(i)); }
inline void stringAppend(std::string &str, long long int i) { str.append(cToSvInt(i)); }

// strings
inline void stringAppend(std::string &str, const char *s) { if(s) str.append(s); }
inline void stringAppend(std::string &str, const std::string &s) { str.append(s); }
inline void stringAppend(std::string &str, std::string_view s) { str.append(s); }
inline void stringAppend(std::string &str, cSv s) { str.append(s); }

inline void stringAppend(std::string &str, const tChannelID &channelID) {
  str.append(cToSvChannel(channelID));
}
template<typename T, typename U, typename... Args>
void stringAppend(std::string &str, const T &n, const U &u, const Args&... args) {
  stringAppend(str, n);
  stringAppend(str, u, args...);
}

// =========================================================
// =========== concatenate =================================
// =========================================================

// deprecated. Use cToSvConcat
inline std::string concatenate() { return std::string(); }
template<typename T> inline std::string concatenate(const T &t) {
  std::string result;
  stringAppend(result, t);
  return result;
}
template<typename... Args>
inline std::string concatenate(const Args&... args) {
  std::string result;
  result.reserve(200);
//stringAppend(result, std::forward<Args>(args)...);
  stringAppend(result, args...);
  return result;
}

// use concat if you need a string with optimized capacity
//   e.g. the string is member of your class
// otherwise, use cToSvConcat
inline std::string concat(cSv s1, cSv s2) {
  std::string result;
  result.reserve(s1.length() + s2.length());
  result.append(s1);
  result.append(s2);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() );
  result.append(s1);
  result.append(s2);
  result.append(s3);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() );
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() );
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() );
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6, cSv s7) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length());
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  result.append(s7);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6, cSv s7, cSv s8) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length());
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  result.append(s7);
  result.append(s8);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6, cSv s7, cSv s8, cSv s9) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length() + s9.length());
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  result.append(s7);
  result.append(s8);
  result.append(s9);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6, cSv s7, cSv s8, cSv s9, cSv s10) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length() + s9.length() + s10.length());
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  result.append(s7);
  result.append(s8);
  result.append(s9);
  result.append(s10);
  return result;
}
inline std::string concat(cSv s1, cSv s2, cSv s3, cSv s4, cSv s5, cSv s6, cSv s7, cSv s8, cSv s9, cSv s10, cSv s11) {
  std::string result;
  result.reserve(s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length() + s9.length() + s10.length() + s11.length());
  result.append(s1);
  result.append(s2);
  result.append(s3);
  result.append(s4);
  result.append(s5);
  result.append(s6);
  result.append(s7);
  result.append(s8);
  result.append(s9);
  result.append(s10);
  result.append(s11);
  return result;
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
