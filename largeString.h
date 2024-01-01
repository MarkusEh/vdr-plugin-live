#ifndef __LARGE_STRING_H
#define __LARGE_STRING_H

#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "stringhelpers.h"

class cLargeString {
  private:
    char m_s_initial[1] = "";
    char *m_s = m_s_initial;
    char *m_string_end = m_s; // *m_string_end = 0; m_string_end must be < m_buffer_end, because m_buffer_end is excluded
    char *m_buffer_end = m_s + 1; // buffer is between [m_s;m_buffer_end), i.e. m_buffer_end is excluded
    size_t m_increaseSize = 0;
    size_t m_maxSize = 0;
    const char *m_nameData;
    int m_nameLen;
    bool m_debugBufferSize = false;
    bool m_endBorrowed = false;
    void enlarge(size_t increaseSize = 0);
    inline void appendLen(size_t len) {  // enure buffer is large enough to append len characters
      if (m_string_end + len < m_buffer_end) return;
      enlarge(m_string_end + len  + 1 - m_buffer_end);
    }
    void setMaxSize() { m_maxSize = std::max(m_maxSize, (size_t)(m_string_end - m_s)); }
    void init(size_t initialSize, size_t increaseSize, bool debugBufferSize);
    cLargeString &append_int(const char *s, size_t len) {
// internal method, no checks (s!=0)
      appendLen(len);
      memcpy(m_string_end, s, len);
      m_string_end += len;
      return *this;
    }
    template<std::size_t N, typename T> cLargeString &appendHex_int(T u) {
      appendLen(N);
      stringhelpers_internal::addCharsHex(m_string_end, N, u);
      m_string_end += N;
      return *this;
    }
  public:
    cLargeString(const cLargeString& o) = delete;
    cLargeString &operator= (const cLargeString &) = delete;
//    cLargeString(cLargeString&& o) = default; // default is wrong, explicit definition required
//    cLargeString &operator= (cLargeString &&) = default; // default is wrong, explicit definition required
    template<std::size_t N>
    cLargeString(const char (&name)[N], size_t initialSize = 0, size_t increaseSize = 0, bool debugBufferSize = false) {
      m_nameData = name;
      m_nameLen = static_cast<int>(N) - 1;
      init(initialSize, increaseSize, debugBufferSize);
    }
    ~cLargeString();
    char *data() { *m_string_end = 0; return m_s; }
    size_t length() const { return m_string_end - m_s; }
    const char *c_str() const { *m_string_end = 0; return m_s; }
    char operator[](size_t i) const { return *(m_s + i); }
    operator cSv() const { return cSv(m_s, m_string_end - m_s); }
    bool empty() const { return m_string_end == m_s; }

    cLargeString &append(char c) {
      appendLen(1);
      *(m_string_end++) = c;
      return *this;
    }
    cLargeString &append(size_t count, char c) {
      appendLen(count);
      memset(m_string_end, c, count);
      m_string_end += count;
      return *this;
    }
    cLargeString &append(const char (&s)[1]) {
// note: every other specific implementation is too slow. memcpy is too fast :)
      return *this;
    }
    template<std::size_t N> cLargeString &append(const char (&s)[N]) {
//      std::cout << "append template!!!!\n";
      appendLen(N-1);
      memcpy(m_string_end, s, N-1);
      m_string_end += N-1;
      return *this;
    }
    cLargeString &append(const char *s, size_t len) {
      if (!s) return *this;
      return append_int(s, len);
    }
    cLargeString &appendS(const char *s) {
      if (!s || !*s) return *this;
      return append_int(s, strlen(s));
    }
    template<typename T, std::enable_if_t<std::is_same<T, const char*>::value, bool> = true>
      cLargeString &append(T s) { return appendS(s); }
    template<typename T, std::enable_if_t<std::is_same<T, char*>::value, bool> = true>
      cLargeString &append(T s) { return appendS(s); }
    cLargeString &append(const std::string &s) { return append_int(s.c_str(), s.length()); }
    cLargeString &append(cSv s) { return append_int(s.data(), s.length()); }
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    cLargeString &append(T i) {
      appendLen(20);
      m_string_end = stringhelpers_internal::itoa(m_string_end, i);
      return *this;
    }
    template<std::size_t N> cLargeString &appendHex(unsigned u) { return appendHex_int<N>(u); }
    template<std::size_t N> cLargeString &appendHex(unsigned long u) { return appendHex_int<N>(u); }
    template<std::size_t N> cLargeString &appendHex(unsigned long long u) { return appendHex_int<N>(u); }
    template<typename... Args> cLargeString &appendFormated(const char *format, Args&&... args) {
      size_t avail = m_buffer_end - m_string_end;
      size_t numNeeded = snprintf(m_string_end, avail, format, std::forward<Args>(args)...);
      if (numNeeded >= avail) {
        appendLen(numNeeded + 1);
        sprintf(m_string_end, format, std::forward<Args>(args)...);
      }
      m_string_end += numNeeded;
      return *this;
    }
    char *borrowEnd(size_t len); // len: upper limit of characters that can be written
    bool finishBorrow(size_t len);  // len: number of actually written characters
    bool finishBorrow();  // number of actually written characters: zero terminated
    void clear();
//    cLargeString &erase(size_t index = 0) { setMaxSize(); m_string_end = std::min(m_string_end, m_s + index); return *this;}
    const char *nameData() const { return m_nameData; }
    int nameLen() const { return m_nameLen; }
};
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
void stringAppend(cLargeString &s, T i) {
  s.append(i);
}

inline void append_csv(cLargeString &str, cSv s1) { str.append(s1); }
template<typename... Args>
inline void append_csv(cLargeString &str, cSv s1, Args&&... args) {
  str.append(s1);
  append_csv(str, std::forward<Args>(args)...);
}

#endif  // __LARGE_STRING_H
