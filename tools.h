#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

// uncomment to debug lock sequence 
// #define DEBUG_LOCK

#include "largeString.h"
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
#include <vdr/channels.h>

#define CONVERT(result, from, fn) \
char result[fn(NULL, from) + 1]; \
fn(result, from);

std::istream& operator>>( std::istream& is, tChannelID& ret );

inline
std::ostream& operator<<( std::ostream& os, tChannelID const& id )
{
	return os << *id.ToString();
}


#if TNTVERSION >= 30000
namespace cxxtools
{
	class SerializationInfo;

	inline void operator<<= (cxxtools::SerializationInfo& si, const tChannelID& id)
	{
//		dsyslog("live: operator<<= called");
	}

	inline void operator>>= (const cxxtools::SerializationInfo& si, tChannelID& id)
	{
//		dsyslog("live: operator>>= called");
	}
}
#endif


namespace vdrlive {
	extern const std::locale g_locale;
	extern const std::collate<char>& g_collate_char;

        void AppendHtmlEscaped(std::string &target, const char* s);
template<class T>
        void AppendHtmlEscapedAndCorrectNonUTF8(T &target, const char* s, const char *end = NULL, bool tooltip = false);
template<class T>
	void AppendTextTruncateOnWord(T &target, const char *text, int max_len, bool tooltip = false);
        void AppendCorrectNonUTF8(std::string &target, const char* s);

        wint_t getNextUtfCodepoint(const char *&p);
        int utf8CodepointIsValid(const char *p); // In case of invalid UTF8, return 0. Otherwise: Number of characters
        wint_t Utf8ToUtf32(const char *&p, int len); // assumes, that uft8 validity checks have already been done. len must be provided. call utf8CodepointIsValid first
	void AppendUtfCodepoint(std::string &target, wint_t codepoint);



        void AppendDuration(cLargeString &target, char const* format, int hours, int minutes );
        void AppendDuration(std::string &target, char const* format, int hours, int minutes );
	std::string FormatDuration( char const* format, int hours, int minutes );

        void AppendDateTime(cLargeString &target, char const* format, time_t time );
        void AppendDateTime(std::string &target, char const* format, time_t time );
	std::string FormatDateTime( char const* format, time_t time );

	std::string StringReplace( std::string const& text, std::string const& substring, std::string const& replacement );

	std::vector<std::string> StringSplit( std::string const& text, char delimiter );

	int StringToInt( std::string const& string, int base = 10 );

	std::string StringWordTruncate(const std::string& input, size_t maxLen, bool& truncated);
	inline std::string StringWordTruncate(const std::string& input, size_t maxLen) { bool dummy; return StringWordTruncate(input, maxLen, dummy); }

        std::string StringEscapeAndBreak(std::string const& input, const char* nl = "<br/>");

	std::string StringFormatBreak(std::string const& input);

	std::string StringTrim(const std::string& str);
	std::string ZeroPad(int number);

	const char *getText(const char *shortText, const char *description);
template<class T>
        void AppendTextMaxLen(T &target, const char *text);

	std::string MD5Hash(std::string const& str);
	std::string xxHash32(std::string const& str);

	time_t GetTimeT(std::string timestring);
	std::string ExpandTimeString(std::string timestring);

	std::string StringUrlEncode( std::string const& input );

	std::string GetXMLValue( std::string const& xml, std::string const& element );

	time_t GetDateFromDatePicker(std::string const& datestring, std::string const& format);
	std::string DatePickerToC(time_t date, std::string const& format);
	std::string intToTimeString(int tm);
	int timeStringToInt(const char *t);
	int timeStringToInt(const std::string &t);
	std::string charToString(const char *s);

	std::string EncodeDomId(std::string const & toEncode, char const * from = ".-:", char const * to = "pmc");
	std::string DecodeDomId(std::string const & toDecode, char const * from = "pmc", char const * to = ".-:");

	std::string FileSystemExchangeChars(std::string const & s, bool ToFileSystem);

	bool MoveDirectory(std::string const & sourceDir, std::string const & targetDir, bool copy = false);

  template<class T> int toCharsU(char *buffer, T i);
  template<class T> int toCharsI(char *buffer, T i);
// notes:
//    return number of characters written to buffer  (don't count 0 terminator)
//    if buffer==NULL: don't write anything, just return the number
//    sizeof(buffer) must be >= this return value + 1 !! This is not checked ....

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
        std::string ScraperImagePath2Live(const std::string &path);

// call the service Id
// return false if there is no scraper plugin, or if the serrvice does not exist
// otherwise, return true
// can be called with Data == Null to check is the service exits
        bool ScraperCallService(const char *Id, void *Data);
} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
