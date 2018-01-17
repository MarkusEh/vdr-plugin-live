#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <istream>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifndef __STL_CONFIG_H
// To get rid of the swap definition in vdr/tools.h
# define __STL_CONFIG_H
#endif
#include <vdr/channels.h>

std::istream& operator>>( std::istream& is, tChannelID& ret );

inline
std::ostream& operator<<( std::ostream& os, tChannelID const& id )
{
	return os << *id.ToString();
}

namespace vdrlive {

	std::string FormatDuration( char const* format, int hours, int minutes );

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

	std::string FileSystemExchangeChars(std::string const & s, bool ToFileSystem);

	bool MoveDirectory(std::string const & sourceDir, std::string const & targetDir, bool copy = false);

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
		private:
			typedef void (ReadLock::*safe_bool)() const;

		public:
            ReadLock(cRwLock& lock, int timeout = 100)
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

} // namespace vdrlive

#endif // VDR_LIVE_TOOLS_H
