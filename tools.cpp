#include <stdexcept>
#include <iomanip>
#include <tnt/ecpp.h>
#include <tnt/htmlescostream.h>
#include <tnt/httprequest.h>
#include <tnt/httpreply.h>
#include "exception.h"
#include "live.h"
#include "setup.h"
#include "tools.h"


using namespace std;
using namespace tnt;

istream& operator>>( istream& is, tChannelID& ret )
{
	if ( is.rdbuf()->in_avail() > 0 ) {
		string line;
		if ( !getline( is, line ) || ( !line.empty() && !( ret = tChannelID::FromString( line.c_str() ) ).Valid() ) )
			is.setstate( ios::badbit );
	}
	return is;
}

namespace vdrlive {

string FormatDateTime( char const* format, time_t time )
{
	struct tm tm_r;
	if ( localtime_r( &time, &tm_r ) == 0 ) {
		ostringstream builder;
		builder << "cannot represent timestamp " << time << " as local time";
		throw runtime_error( builder.str() );
	}

	char result[ 256 ];
	if ( strftime( result, sizeof( result ), format, &tm_r ) == 0 ) {
		ostringstream builder;
		builder << "representation of timestamp " << time << " exceeds " << sizeof( result ) << " bytes";
		throw runtime_error( builder.str() );
	}
	return result;
}

string StringReplace( string const& text, string const& substring, string const& replacement )
{
	string result = text;
	string::size_type pos = 0;
	while ( ( pos = result.find( substring, pos ) ) != string::npos ) {
		result.replace( pos, substring.length(), replacement );
		pos += replacement.length();
	}
	return result;
}

vector< string > StringSplit( string const& text, char delimiter )
{
	vector< string > result;
	string::size_type last = 0, pos;
	while ( ( pos = text.find( delimiter, last ) ) != string::npos ) {
		result.push_back( text.substr( last, pos - last ) );
		last = pos + 1;
	}
	if ( last < text.length() )
		result.push_back( text.substr( last ) );
	return result;
}

int StringToInt( std::string const& string, int base )
{
	char* end;
	int result = strtol( string.c_str(), &end, base );
	if ( *end == '\0' )
		return result;
	return 0;
}

string StringRepeat(int times, const string& input)
{
	string result;
	for (int i = 0; i < times; i++) {
		result += input;
	}
	return result;
}

string StringWordTruncate(const string& input, size_t maxLen, bool& truncated)
{
	if (input.length() <= maxLen)
	{
		return input;
	}
	truncated = true;
	string result = input.substr(0, maxLen);
	size_t pos = result.find_last_of(" \t,;:.\n?!'\"/\\()[]{}*+-");
	return result.substr(0, pos);
}

string StringFormatBreak(string const& input)
{
	return StringReplace( input, "\n", "<br/>" );
}

string StringEscapeAndBreak( string const& input )
{
	stringstream plainBuilder;
	HtmlEscOstream builder( plainBuilder );
	builder << input;
	return StringReplace( plainBuilder.str(), "\n", "<br/>" );
}

string StringTrim(string const& str)
{
   string res = str;
   string::size_type pos = res.find_last_not_of(' ');
   if(pos != string::npos) {
      res.erase(pos + 1);
      pos = res.find_first_not_of(' ');
      if(pos != string::npos) res.erase(0, pos);
   }
   else res.erase(res.begin(), res.end());
   return res;
}

string ZeroPad(int number)
{
   ostringstream os;
   os << setw(2) << setfill('0') << number;
   return os.str();
}


} // namespace vdrlive
