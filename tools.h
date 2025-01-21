#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

// uncomment to debug lock sequence
// #define DEBUG_LOCK

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <istream>
#include <sstream>
#include <stdexcept>
#include <vector>

#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
  #include "cxxtools/serializationinfo.h"
#endif

#ifndef DISABLE_TEMPLATES_COLLIDING_WITH_STL
// To get rid of the swap definition in vdr/tools.h
#define DISABLE_TEMPLATES_COLLIDING_WITH_STL
#endif
#include "stringhelpers.h"
#include "xxhash.h"
#include <vdr/channels.h>

// ================ XXH128_hash_t  ============================================
inline cToSvHex<32>& operator<<(cToSvHex<32> &h, const XXH128_hash_t &value) {
  stringhelpers_internal::addCharsHex(h.m_buffer,    16, value.high64);
  stringhelpers_internal::addCharsHex(h.m_buffer+16, 16, value.low64);
  return h;
}
template <size_t N>
inline cToSvConcat<N>& operator<<(cToSvConcat<N>& s, const XXH128_hash_t &value) {
  s.appendHex(value.high64);
  s.appendHex(value.low64);
  return s;
}

// ================ Channels ============================================

template <size_t N>
inline cToSvConcat<N> &stringAppendChannel(cToSvConcat<N> &target, const tChannelID &channelID, char point = '.', char minus = '-') {
  const int st_Mask = 0xFF000000;
  const int st_Pos  = 0x0000FFFF;
  target.concat((char) ((channelID.Source() & st_Mask) >> 24));
  if (int16_t n = channelID.Source() & st_Pos) {
    char ew = 'E';
    if (n < 0) {
      ew = 'W';
      n = -n;
    }
    uint16_t q = (uint16_t)n / 10;
    target.concat(q, point, (char)((uint16_t)n - 10*q + '0'), ew);
  }
  target.concat(minus, channelID.Nid(), minus, channelID.Tid(), minus, channelID.Sid());
  if (channelID.Rid() ) target.concat(minus, channelID.Rid() );
  return target;
}
template <size_t N>
inline cToSvConcat<N>& operator<<(cToSvConcat<N>& s, const tChannelID &channelID) {
  return stringAppendChannel(s, channelID);
}
inline void stringAppend(std::string &str, const tChannelID &channelID) {
  str.append(cToSvConcat(channelID));
}

template<class T, std::enable_if_t<std::is_same_v<T, tChannelID>, bool> = true>
inline T lexical_cast(cSv sv, T returnOnError = T(), const char *context = nullptr) {
  char buf[sv.length()+1];
  buf[sv.length()] = 0;
  memcpy(buf, sv.data(), sv.length());
  tChannelID ret = tChannelID::FromString(buf);
  if (context && !ret.Valid() )
    isyslog(PLUGIN_NAME_I18N ": WARNING, converted \"%.*s\" to invalid tChannelID, context %s", (int)sv.length(), sv.data(), context);
  return ret;
}

inline std::ostream& operator<<( std::ostream& os, tChannelID const& id ) {
  return os << cToSvConcat(id);
}
std::istream& operator>>( std::istream& is, tChannelID& ret );

namespace vdrlive {
  extern const std::locale g_locale;
  extern const std::collate<char>& g_collate_char;

template <size_t N>
inline cToSvConcat<N>& AppendHtmlEscapedAndCorrectNonUTF8(cToSvConcat<N>& target, cSv text, bool tooltip = false) {
  size_t i = 0;                 // number of not yet appended chars
  const char* notAppended = text.data();  // position of the first character which is not yet appended
  for (size_t pos = 0; pos < text.length(); ++pos) {
    if ((unsigned char)text[pos] <= '\'') {
      switch(text[pos]) {
        case '\t': target.append(notAppended, i); target.append("&tab;");  notAppended += i + 1; i = 0; break;
        case '\n':
        case '\r':
//                 target.append(notAppended, i); target.append("&lt;br/&gt;");   notAppended += i + 1; i = 0; break;
                   target.append(notAppended, i); target.append("<br/>");  notAppended += i + 1; i = 0; break;
        case '&':  target.append(notAppended, i); target.append("&amp;");  notAppended += i + 1; i = 0; break;
        case '\"': target.append(notAppended, i); target.append("&quot;"); notAppended += i + 1; i = 0; break;
        case '\'': target.append(notAppended, i); target.append("&apos;"); notAppended += i + 1; i = 0; break;
        case ' ':
        case '!':
        case '#':
        case '$':
        case '%':
          ++i; break;  // just append these characters, no encoding
        default:  // replace control characters with ?
          target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0; break;
        }
      continue;
    }
    if ((unsigned char)text[pos] <= '~') {
      switch(text[pos]) {
        case '\\': target.append(notAppended, i); target.append("&bsol;"); notAppended += i + 1; i = 0; break;
        case '<':  target.append(notAppended, i); target.append("&lt;");   notAppended += i + 1; i = 0; break;
        case '>':  target.append(notAppended, i); target.append("&gt;");   notAppended += i + 1; i = 0; break;
        default:
          ++i; break;  // just append these characters, no encoding
        }
      continue;
    }
    if (text[pos] == 127) { // replace control characters with ?
      target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0;
      continue;
    }
    int l = text.utf8CodepointIsValid(pos);
    if (l == 0) {
      // invalid UTF8, replace with ?
      target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0;
      continue;
    }
    if (l == 3 && text[pos] == '\xEE' && text[pos+1] == '\x80') {
      target.append(notAppended, i);
      switch (text[pos+2]) {
// mapping of VDR private-use symbols onto well-known UTF symbols
        case '\x80':
// reversion symbol (counter-clockwise arrow)
          target.append("\u21BA");
          break;
        case '\x82':
// U+E002: folder symbol
          target.append("\U0001F4C1");
          break;
        case '\x83':
// U+E003: non-breaking space
          target.append("\u00A0");
          break;
        case '\x8B':
// U+E00B: recording symbol
          target.append("\u00AE");
          break;
        case '\x8C':
// U+E00C: timer symbol (full coverage)
          target.append("\u23F2");
          break;
        case '\x91':
// U+E011: continuation symbol (clockwise arrow)
          target.append("\u21BB");
          break;
        case '\x92':
// U+E012: running symbol
          target.append("\u25B6");
          break;
        case '\x93':
// U+E013: VPS symbol
          target.append("\U0001F185");
          break;
        case '\x94':
// U+E014: partial timer symbol
          target.append("\u26AC");
          break;
        case '\x95':
// U+E015: inactive timer symbol
          target.append("\u29B8");
          break;
        default:
          target.append(text.substr(pos, 3));
          break;
      }
      notAppended += i + 3; i = 0;
      pos += 2;
      continue;
    }
    i += l;
    pos += l-1;
  }
  target.append(notAppended, i);
  return target;
}

template <size_t N>
inline cToSvConcat<N>& AppendQuoteEscapedAndCorrectNonUTF8(cToSvConcat<N>& target, cSv text) {
  size_t i = 0;                 // number of not yet appended chars
  const char* notAppended = text.data();  // position of the first character which is not yet appended
  for (size_t pos = 0; pos < text.length(); ++pos) {
    if ((unsigned char)text[pos] <= '\"') {
      switch(text[pos]) {
        case '\t': target.append(notAppended, i); target.append("\\t"); notAppended += i + 1; i = 0; break;
        case '\n': target.append(notAppended, i); target.append("\\n"); notAppended += i + 1; i = 0; break;
        case '\r': target.append(notAppended, i); target.append("\\r"); notAppended += i + 1; i = 0; break;
        case '\"': target.append(notAppended, i); target.append("\\"); notAppended += i; i = 1; break;
        case ' ':
        case '!':
          ++i; break;  // just append these characters, no encoding
        default:  // replace control characters with ?
          target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0; break;
        }
      continue;
    }
    if ((unsigned char)text[pos] <= '~') {
      if (text[pos]  == '\\') {
        target.append(notAppended, i); target.append("\\"); notAppended += i; i = 1; // this results in appending two backslashs
      } else {
        ++i; // just append these characters, no encoding
      }
      continue;
    }
    if (text[pos] == 127) { // replace control characters with ?
      target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0;
      continue;
    }
    int l = text.utf8CodepointIsValid(pos);
    if (l == 0) {
      // invalid UTF8, replace with ?
      target.append(notAppended, i); target.append("?"); notAppended += i + 1; i = 0;
    } else {
      i += l;
      pos += l-1;
    }
  }
  target.append(notAppended, i);
  return target;
}

cSv StringWordTruncate(cSv text, size_t maxLen, bool& truncated);
inline cSv StringWordTruncate(cSv text, size_t maxLen) { bool dummy; return StringWordTruncate(text, maxLen, dummy); }

template <size_t N>
inline cToSvConcat<N>& AppendTextTruncateOnWord(cToSvConcat<N>& target, cSv text, int max_len, bool tooltip = false) {
// append text to target, but only up to max_len characters. If such truncation is required, truncate at ' ' \n, ... and similar
// escape HTML characters, and correct invalid UTF8
  bool truncated;
  AppendHtmlEscapedAndCorrectNonUTF8(target, StringWordTruncate(text, max_len, truncated), tooltip);
  if (truncated) target.append(" ...");
  return target;
}

template<size_t N>
inline cToSvConcat<N>& AppendDuration(cToSvConcat<N>& target, char const* format, int duration) {
  int minutes = (duration + 30) / 60;
  int hours = minutes / 60;
  minutes %= 60;
  target.appendFormated(format, hours, minutes);
  return target;
}

  std::string FormatDuration( char const* format, int duration );

  cSv StringTrim(cSv str);

  std::string MD5Hash(std::string const& str);

  class cToSvXxHash32: public cToSvHex<8> {
    public:
      cToSvXxHash32(XXH32_hash_t value): cToSvHex<8>::cToSvHex(value) {}
      cToSvXxHash32(cSv str): cToSvHex<8>::cToSvHex(XXH32(str.data(), str.length(), 20)) {}
  };

  XXH64_hash_t parse_hex_64(cSv str);
  class cToSvXxHash64: public cToSvHex<16> {
    public:
      cToSvXxHash64(XXH64_hash_t value): cToSvHex<16>::cToSvHex(value) {}
      cToSvXxHash64(cSv str): cToSvHex<16>::cToSvHex(XXH3_64bits(str.data(), str.length() )) {}
  };

  XXH128_hash_t parse_hex_128(cSv str);
  class cToSvXxHash128: public cToSvHex<32> {
    public:
      cToSvXxHash128(XXH128_hash_t value): cToSvHex<32>::cToSvHex(value) { }
      cToSvXxHash128(cSv str): cToSvHex<32>::cToSvHex(XXH3_128bits(str.data(), str.length() )) {}
  };

  time_t GetTimeT(cSv timestring); // timestring in HH:MM
  std::string ExpandTimeString(std::string timestring);

  time_t GetDateFromDatePicker(cSv datestring, cSv format);
  std::string DatePickerToC(time_t date, cSv format);
  std::string intToTimeString(int tm);
  int timeStringToInt(const char *t);
  int timeStringToInt(const std::string &t);

  void EncodeDomId(char *toEncode_s, char *toEncode_e, char const * from = ".-:", char const * to = "pmc");
  inline void DecodeDomId(char *toDecode_s, char *toDecode_e, char const * from = "pmc", char const * to = ".-:") {
    EncodeDomId(toDecode_s, toDecode_e, from, to);
  }

  std::string EncodeDomId(cSv toEncode, char const * from = ".-:", char const * to = "pmc");
  inline std::string DecodeDomId(cSv toDecode, char const * from = "pmc", char const * to = ".-:") {
    return EncodeDomId(toDecode, from, to);
  }

  std::string FileSystemExchangeChars(cSv s, bool ToFileSystem);

  bool MoveDirectory(cSv sourceDir, cSv targetDir, bool copy = false);

// methods for scraper **************************************

// tool for images returned by Tvscraper or scraper2vdr:
// convert path (valid in local file system) to path which can be used by live (in browser) to access the image
// Note: final browser path is: "/tvscraper/" + ScraperImagePath2Live(...)
  cSv ScraperImagePath2Live(cSv path);

// call the service Id
// return false if there is no scraper plugin, or if the service does not exist
// otherwise, return true
// can be called with Data == Null to check is the service exits
  bool ScraperCallService(const char *Id, void *Data);
} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
