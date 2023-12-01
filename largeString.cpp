#include <string.h>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <vdr/plugin.h>
#include "largeString.h"

void cLargeString::init(size_t initialSize, size_t increaseSize, bool debugBufferSize) {
  m_debugBufferSize = debugBufferSize;
  if (increaseSize > 0) m_increaseSize = increaseSize;
  else m_increaseSize = std::max((size_t)100, initialSize / 2);
  if (initialSize == 0) return;
  m_s = (char *) malloc(initialSize * sizeof(char));
  if (!m_s) {
    esyslog("cLargeString::init, ERROR out of memory, name = %.*s, initialSize = %zu", nameLen(), nameData(), initialSize);
    throw std::runtime_error("cLargeString::cLargeString, ERROR out of memory");
    return;
  }
  m_buffer_end = m_s + initialSize;
  m_string_end = m_s;
}

cLargeString::~cLargeString() {
  if (m_s == m_s_initial) return;
  free (m_s);
  setMaxSize();
  size_t buffer_len = m_buffer_end - m_s;
  if (m_debugBufferSize && buffer_len > m_maxSize * 2) esyslog("cLargeString::cLargeString WARNING too large buffer, name = %.*s, buffer %zu, len %zu", nameLen(), nameData(), buffer_len, m_maxSize);
}

void cLargeString::clear() {
  setMaxSize();
  m_string_end = m_s;
  m_endBorrowed = false;
}

char *cLargeString::borrowEnd(size_t len) {
  appendLen(len);
  m_endBorrowed = true;
  return m_string_end;
}
bool cLargeString::finishBorrow(size_t len) {
  if (!m_endBorrowed) return true;
  m_endBorrowed = false;
  if (m_string_end + len >= m_buffer_end) {
    esyslog("cLargeString::finishBorrow(size_t len), ERROR name = %.*s, len %zu too large, available %zu", nameLen(), nameData(), len, m_buffer_end - m_string_end - 1);
    m_string_end = m_buffer_end - 1;
    return false;
  }
  m_string_end = m_string_end + len;
  return true;
}

bool cLargeString::finishBorrow() {
  if (!m_endBorrowed) return true;
  m_endBorrowed = false;
  char *end = strchr(m_string_end, 0);
  if (!end) return false;
  if (end >= m_buffer_end) {
    esyslog("cLargeString::finishBorrow(), ERROR name = %.*s, end >= m_buffer_end, available %zu", nameLen(), nameData(), m_buffer_end - m_string_end - 1);
    m_string_end = m_buffer_end - 1;
    return false;
  }
  m_string_end = end;
  return true;
}

cLargeString &cLargeString::append(char c) {
  appendLen(1);
  *(m_string_end++) = c;
  return *this;
}

cLargeString &cLargeString::append(int i) {
  if (i < 0) {
    appendLen(2);
    *(m_string_end++) = '-';
    i *= -1;
  }
  if (i < 10) return append((char)('0' + i));
  char buf[21]; // unsigned int 64: max. 20. (18446744073709551615) signed int64: max. 19 (+ sign)
  char *bufferEnd = buf+20;
  *bufferEnd = 0;
  for (; i; i /= 10) *(--bufferEnd) = '0' + (i%10);
  return appendS(bufferEnd);
}

cLargeString &cLargeString::append(const char *s, size_t len) {
  if (!s || len == 0) return *this;
  appendLen(len);
  memcpy(m_string_end, s, len);
  m_string_end += len;
  return *this;
}

cLargeString &cLargeString::appendS(const char *s) {
  if (!s || !*s) return *this;
  size_t len = strlen(s);
  appendLen(len);
  memcpy(m_string_end, s, len);
  m_string_end += len;
  return *this;
}
void cLargeString::enlarge(size_t increaseSize) {
  increaseSize = std::max(increaseSize, m_increaseSize);
  increaseSize = std::max(increaseSize, (size_t)((m_buffer_end - m_s)/2) );
  size_t stringLength = m_string_end - m_s;
  size_t newSize = m_buffer_end - m_s + increaseSize;
  if (m_s == m_s_initial)
    m_s = (char *) malloc(newSize * sizeof(char));
  else {
    if (m_debugBufferSize) esyslog("cLargeString::cLargeString, WARNING realloc required!!!, name = %.*s, new Size = %zu", nameLen(), nameData(), newSize);
    m_s = (char *)realloc(m_s, newSize * sizeof(char));
  }
  if (!m_s) {
    esyslog("cLargeString::cLargeString, ERROR out of memory, name = %.*s, new Size = %zu", nameLen(), nameData(), newSize);
    throw std::runtime_error("cLargeString::cLargeString, ERROR out of memory (enlarge)");
    return;
  }
  m_buffer_end = m_s + newSize;
  m_string_end = m_s + stringLength;
}

std::string cLargeString::substr(size_t pos, size_t count) const {
  if (pos >= length() ) return "";
  std::string result(m_s + pos, std::min(length() - pos, count) );
  for (char &si: result) if (si == 0) si = '%';
  return result;
}
