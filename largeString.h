#ifndef __LARGE_STRING_H
#define __LARGE_STRING_H

#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

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
    void loadFile(const char *filename, bool *exists);
  public:
    cLargeString(const cLargeString& o) = delete;
    cLargeString &operator= (const cLargeString &) = delete;
//   cLargeString(cLargeString&& o) = default;  // default is wrong, explicit definition required
//   cLargeString &operator= (cLargeString &&) = default;  // default is wrong, explicit definition required
    template<std::size_t N>
    cLargeString(const char (&name)[N], size_t initialSize = 0, size_t increaseSize = 0, bool debugBufferSize = false) {
      m_nameData = name;
      m_nameLen = static_cast<int>(N) - 1;
      init(initialSize, increaseSize, debugBufferSize);
    }
    template<std::size_t N>
    cLargeString(const char (&name)[N], const char *filename, bool *exists = NULL) {
      m_nameData = name;
      m_nameLen = static_cast<int>(N) - 1;
      loadFile(filename, exists);
    }
    ~cLargeString();
    char *data() { *m_string_end = 0; return m_s; }
    const char *c_str() const { *m_string_end = 0; return m_s; }
    cLargeString &append(const char (&s)[1]) {
// note: every other specific implementation is too slow. memcpy is too fast :)
//      std::cout << "append template xx1xx !!!!\n";
      return *this;
    }
    template<std::size_t N> cLargeString &append(const char (&s)[N]) {
//      std::cout << "append template!!!!\n";
      appendLen(N-1);
      memcpy(m_string_end, s, N-1);
      m_string_end += N-1;
      return *this;
    }
    cLargeString &append(char c);
    cLargeString &append(const char *s, size_t len);
    cLargeString &append(const std::string &s) { return append(s.c_str(), s.length()); }
    cLargeString &append(int i);
    cLargeString &appendS(const char *s);
    template<typename... Args> cLargeString &appendFormated(char const* format, Args&&... args) {
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
    inline size_t length() const { return m_string_end - m_s; }
    inline bool empty() const { return m_string_end == m_s; }
    cLargeString &erase(size_t index = 0) { setMaxSize(); m_string_end = std::min(m_string_end, m_s + index); return *this;}
    char operator[](size_t i) const { return *(m_s + i); }
    const char *nameData() const { return m_nameData; }
    int nameLen() const { return m_nameLen; }
};

#endif  // __LARGE_STRING_H
