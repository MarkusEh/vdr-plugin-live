#ifndef VDR_LIVE_TOOLS_H
#define VDR_LIVE_TOOLS_H

#include <ctime>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vdr/thread.h>

namespace vdrlive {

std::string FormatDateTime( char const* format, time_t time );
std::string StringReplace( std::string const& text, std::string const& substring, std::string const& replacement );
std::vector< std::string > StringSplit( std::string const& text, char delimiter );
int StringToInt( std::string const& string, int base = 10 );
std::string StringRepeat(int times, const std::string& input);
std::string StringWordTruncate(const std::string& input, size_t maxLen, bool& truncated);

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
