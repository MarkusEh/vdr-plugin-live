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
  #include <cxxtools/log.h>  // must be loaded before any vdr include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
  #include "cxxtools/serializationinfo.h"
#endif

#ifndef DISABLE_TEMPLATES_COLLIDING_WITH_STL
// To get rid of the swap definition in vdr/tools.h
#define DISABLE_TEMPLATES_COLLIDING_WITH_STL
#endif
#include "xxhash.h"
#include <vdr/channels.h>
#include "stringhelpers.h"
#include "largeString.h"

std::istream& operator>>( std::istream& is, tChannelID& ret );

inline
std::ostream& operator<<( std::ostream& os, tChannelID const& id )
{
	return os << cToSvConcat(id);
}

template<typename... Args> void stringAppendFormated(cLargeString &target, const char *format, Args&&... args) {
  target.appendFormated(format, std::forward<Args>(args)...);
}

namespace vdrlive {
	extern const std::locale g_locale;
	extern const std::collate<char>& g_collate_char;

  void AppendHtmlEscaped(std::string &target, const char* s);
template<class T>
  void AppendHtmlEscapedAndCorrectNonUTF8(T &target, const char* s, const char *end = NULL, bool tooltip = false);
template<class T>
  inline void AppendHtmlEscapedAndCorrectNonUTF8(T &target, cSv s, bool tooltip = false) {
    AppendHtmlEscapedAndCorrectNonUTF8(target, s.data(), s.data() + s.length(), tooltip);
  }

template<class T>
	void AppendTextTruncateOnWord(T &target, const char *text, int max_len, bool tooltip = false);

  template<typename T> void AppendDuration(T &target, char const* format, int duration);
	std::string FormatDuration( char const* format, int duration );

  void AppendDateTime(cLargeString &target, char const* format, time_t time );
  void AppendDateTime(std::string &target, char const* format, time_t time );
	std::string FormatDateTime( char const* format, time_t time );

	std::string StringReplace(cSv text, cSv substring, cSv replacement );

	std::vector<std::string> StringSplit(cSv text, char delimiter );

	cSv StringWordTruncate(cSv input, size_t maxLen, bool& truncated);
	inline cSv StringWordTruncate(cSv input, size_t maxLen) { bool dummy; return StringWordTruncate(input, maxLen, dummy); }

  std::string StringEscapeAndBreak(cSv input, const char* nl = "<br/>");
	std::string StringFormatBreak(cSv input);
	cSv StringTrim(cSv str);

template<class T>
  void AppendTextMaxLen(T &target, const char *text);

  std::string MD5Hash(std::string const& str);
	std::string xxHash32(cSv str);

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
      cToSvXxHash128(XXH128_hash_t value): cToSvHex<32>::cToSvHex() { setb(value); }
      cToSvXxHash128(cSv str): cToSvHex<32>::cToSvHex() { setb(XXH3_128bits(str.data(), str.length() )); }
    private:
      void setb(XXH128_hash_t value) {
        stringhelpers_internal::addCharsHex(m_buffer,    16, value.high64);
        stringhelpers_internal::addCharsHex(m_buffer+16, 16, value.low64);
      }
  };

	time_t GetTimeT(std::string timestring); // timestring in HH:MM
	std::string ExpandTimeString(std::string timestring);

	std::string StringUrlEncode(cSv input);

	time_t GetDateFromDatePicker(cSv datestring, cSv format);
	std::string DatePickerToC(time_t date, cSv format);
	std::string intToTimeString(int tm);
	int timeStringToInt(const char *t);
	int timeStringToInt(const std::string &t);

	std::string EncodeDomId(cSv toEncode, char const * from = ".-:", char const * to = "pmc");
	std::string DecodeDomId(cSv toDecode, char const * from = "pmc", char const * to = ".-:");

	std::string FileSystemExchangeChars(cSv s, bool ToFileSystem);

	bool MoveDirectory(cSv sourceDir, cSv targetDir, bool copy = false);

	struct bad_lexical_cast: std::runtime_error
	{
		bad_lexical_cast(): std::runtime_error( "bad lexical cast" ) {}
	};

	template<typename To, typename From>
	To lexical_cast( From const& from )
	{
		std::stringstream parser;
		parser << from;
		To result;
		parser >> result;
		if ( !parser )
			throw bad_lexical_cast();
		return result;
	}

	template<typename From>
	std::string ConvertToString( From const& from, std::locale const& loc = g_locale )
	{
		std::ostringstream parser;
		parser.imbue( loc );
		parser << from;
		return parser.str();
	}

	class ReadLock
	{
		private:
			typedef void (ReadLock::*safe_bool)() const;

		public:
            explicit ReadLock(cRwLock& lock, int timeout = 100)
                : m_lock(lock)
                , m_locked(false)
            {
                if (m_lock.Lock( false, timeout ))
                    m_locked = true;
            }

			~ReadLock()
			{
				if (m_locked)
					m_lock.Unlock();
			}

			operator safe_bool() const
			{
				return m_locked ? &ReadLock::safe_bool_idiom : 0;
			}

		private:
			ReadLock(ReadLock const&);

			cRwLock& m_lock;
			bool m_locked;

			void safe_bool_idiom() const {}
	};

// methods for scraper **************************************

// tool for images returned by tvscraper or scraper2vdr:
// convert path (valid in local file system) to path which can be used by live (in browser) to access the image
// Note: final browser path is: "/tvscraper/" + ScraperImagePath2Live(...)
  cSv ScraperImagePath2Live(cSv path);

// call the service Id
// return false if there is no scraper plugin, or if the serrvice does not exist
// otherwise, return true
// can be called with Data == Null to check is the service exits
  bool ScraperCallService(const char *Id, void *Data);
} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
