#include <string.h>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include "largeString.h"
#include <vdr/plugin.h>

cLargeString::cLargeString(const char *name, size_t initialSize, size_t increaseSize, bool debugBufferSize) {
  if (name) m_name = name;
  m_debugBufferSize = debugBufferSize;
  if (initialSize <= 0) {
    esyslog("cLargeString::cLargeString, ERROR name = %s, initialSize = %zu", m_name.c_str(), initialSize);
    initialSize = 100;
  }
  if (increaseSize > 0) m_increaseSize = increaseSize;
  else m_increaseSize = std::max((size_t)10, initialSize / 2);
  m_s = (char *) malloc(initialSize * sizeof(char));
  if (!m_s) {
    esyslog("cLargeString::cLargeString, ERROR out of memory, name = %s, initialSize = %zu", m_name.c_str(), initialSize);
    throw std::runtime_error("cLargeString::cLargeString, ERROR out of memory");
    return;
  }
  m_buffer_end = m_s + initialSize;
  m_string_end = m_s;
}
cLargeString::~cLargeString() {
  if (!m_s) return;
  free (m_s);
  setMaxSize();
  size_t buffer_len = m_buffer_end - m_s;
  if (m_debugBufferSize && buffer_len > m_maxSize * 2) esyslog("cLargeString::cLargeString WARNING too large buffer, name = %s, buffer %zu, len %zu", m_name.c_str(), buffer_len, m_maxSize);
}

bool cLargeString::clear() {
  if (!m_s) return false;
  setMaxSize();
  m_string_end = m_s;
  m_endBorrowed = false;
  return true;
}

char *cLargeString::borrowEnd(size_t len) {
  if (!m_s) return NULL;
  if (!appendLen(len)) return NULL;
  m_endBorrowed = true;
  return m_string_end;
}
bool cLargeString::finishBorrow(size_t len) {
  if (!m_s) return false;
  if (!m_endBorrowed) return true;
  m_endBorrowed = false;
  if (m_string_end + len >= m_buffer_end) {
    esyslog("cLargeString::finishBorrow(size_t len), ERROR name = %s, len %zu too large, available %zu", m_name.c_str(), len, m_buffer_end - m_string_end - 1);
    m_string_end = m_buffer_end - 1;
    return false;
  }
  m_string_end = m_string_end + len;
  return true;
}

bool cLargeString::finishBorrow() {
  if (!m_s) return false;
  if (!m_endBorrowed) return true;
  m_endBorrowed = false;
  char *end = strchr(m_string_end, 0);
  if (!end) return false;
  if (end >= m_buffer_end) {
    esyslog("cLargeString::finishBorrow(), ERROR name = %s, end >= m_buffer_end, available %zu", m_name.c_str(), m_buffer_end - m_string_end - 1);
    m_string_end = m_buffer_end - 1;
    return false;
  }
  m_string_end = end;
  return true;
}

bool cLargeString::append(char c) {
  if (!m_s) return false;
  if (!appendLen(1)) return false;
  *(m_string_end++) = c;
  return true;
}

bool cLargeString::append(int i) {
  if (!m_s) return false;
  if (i < 0) {
    if(!appendLen(2)) return false;
    *(m_string_end++) = '-';
    i *= -1;
  }
  if (i < 10) return append((char)('0' + i));
  char *buf;
  for (int j = i; j > 0;) {
    for (j = i, buf = m_buffer_end - 2; j > 0 && buf >= m_string_end; j /= 10, buf--) *buf = j%10 + '0';
    if (j > 0 && !enlarge()) return false;
  }
  if (buf >= m_string_end) for (buf++; buf < m_buffer_end - 1;m_string_end++, buf++) *m_string_end = *buf;
  else m_string_end = m_buffer_end - 1;
  return true;
}

bool cLargeString::appendLen(size_t len) {
  char *newStringEnd = m_string_end + len;
  if (newStringEnd < m_buffer_end) return true;
  return enlarge(newStringEnd + 1 - m_buffer_end);
}

bool cLargeString::append(const char *s, size_t len) {
  if (!m_s) return false;
  if (!s || len == 0) return true;
  if (!appendLen(len)) return false;
  for (char *newStringEnd = m_string_end + len; m_string_end < newStringEnd; s++, m_string_end++) *m_string_end = *s;
  return true;
}

bool cLargeString::append(const char *s) {
  if (!m_s) return false;
  if (!s || !*s) return true;
  for (; *s && m_string_end < m_buffer_end; s++, m_string_end++) *m_string_end = *s;
  while(m_string_end == m_buffer_end) {
    if (!enlarge()) return false;
    for (; *s && m_string_end < m_buffer_end; s++, m_string_end++) *m_string_end = *s;
  }
  return true;
}

bool cLargeString::enlarge(size_t increaseSize) {
  if (!m_s) return false;
  increaseSize = std::max(increaseSize, m_increaseSize);
  increaseSize = std::max(increaseSize, (size_t)((m_buffer_end - m_s)/2) );
  size_t stringLength = m_string_end - m_s;
  size_t newSize = m_buffer_end - m_s + increaseSize;
  if (m_debugBufferSize) esyslog("cLargeString::cLargeString, WARNING realloc required!!!, name = %s, new Size = %zu", m_name.c_str(), newSize);
  char *tmp = (char *)realloc(m_s, newSize * sizeof(char));
  if (!tmp) {
    esyslog("cLargeString::cLargeString, ERROR out of memory, name = %s, new Size = %zu", m_name.c_str(), newSize);
    throw std::runtime_error("cLargeString::cLargeString, ERROR out of memory (enlarge)");
    return false;
  }
  m_s = tmp;
  m_buffer_end = m_s + newSize;
  m_string_end = m_s + stringLength;
  return true;
}
