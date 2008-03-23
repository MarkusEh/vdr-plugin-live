#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

#include <ctime>
#include <istream>
#include <locale>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vdr/channels.h>
#include <vdr/thread.h>

std::istream& operator>>( std::istream& is, tChannelID& ret );

inline
std::ostream& operator<<( std::ostream& os, tChannelID const& id )
{
	return os << *id.ToString();
}

namespace vdrlive {

	std::string FormatDateTime( char const* format, time_t time );

	std::string StringReplace( std::string const& text, std::string const& substring, std::string const& replacement );

	std::vector< std::string > StringSplit( std::string const& text, char delimiter );

	int StringToInt( std::string const& string, int base = 10 );

	std::string StringRepeat(int times, const std::string& input);

	std::string StringWordTruncate(const std::string& input, size_t maxLen, bool& truncated);
	inline std::string StringWordTruncate(const std::string& input, size_t maxLen) { bool dummy; return StringWordTruncate(input, maxLen, dummy); }

	std::string StringEscapeAndBreak( std::string const& input );

	std::string StringFormatBreak(std::string const& input);

	std::string StringTrim(const std::string& str);
	std::string ZeroPad(int number);

	std::string MD5Hash(std::string const& str);

	time_t GetTimeT(std::string timestring);
	std::string ExpandTimeString(std::string timestring);

	std::string StringUrlEncode( std::string const& input );

	std::string GetXMLValue( std::string const& xml, std::string const& element );

	time_t GetDateFromDatePicker(std::string const& datestring, std::string const& format);
	std::string DatePickerToC(time_t date, std::string const& format);

	std::string EncodeDomId(std::string const & toEncode, char const * from = ".-:", char const * to = "pmc");
	std::string DecodeDomId(std::string const & toDecode, char const * from = "pmc", char const * to = ".-:");

	struct bad_lexical_cast: std::runtime_error
	{
			bad_lexical_cast(): std::runtime_error( "bad lexical cast" ) {}
	};

	template< typename To, typename From >
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

	template< typename From >
	std::string ConvertToString( From const& from, std::locale const& loc = std::locale() )
	{
		std::ostringstream parser;
		parser.imbue( loc );
		parser << from;
		return parser.str();
	}

	class ReadLock
	{
		public:
			ReadLock( cRwLock& lock, int timeout = 100 ): m_lock( lock ), m_locked( false ) { if ( m_lock.Lock( false, timeout ) ) m_locked = true; }
			~ReadLock() { if ( m_locked ) m_lock.Unlock(); }

			operator bool() { return m_locked; }
			bool operator!() { return !m_locked; }

		private:
			ReadLock( ReadLock const& );

			cRwLock& m_lock;
			bool m_locked;
	};

} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
