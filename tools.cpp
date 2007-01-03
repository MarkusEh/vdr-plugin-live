#include <sstream>
#include <stdexcept>
#include "tools.h"

namespace vdrlive {

using namespace std;
	
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

}
