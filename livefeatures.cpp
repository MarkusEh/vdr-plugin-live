#include "livefeatures.h"
#include "tools.h"

namespace vdrlive {

using namespace std;

SplitVersion::SplitVersion( string version )
	: m_version( 0 )
{
	static const int factors[] = { 100000000, 1000000, 1000, 1 };
	
	size_t pos = version.find('-');
	if ( pos != string::npos ) {
		m_suffix = version.substr( pos + 1 );
		version.erase( pos );
	}
	vector< string > parts = StringSplit( version, '.' );
	for ( size_t i = 0; i < parts.size() && i < sizeof(factors)/sizeof(factors[0]); ++i ) {
		m_version += atoi( parts[ i ].c_str() ) * factors[ i ];
	}
}

bool SplitVersion::operator<( const SplitVersion& right ) const
{
	if ( m_version == right.m_version ) {
		if ( m_suffix.empty() ) return false;
		if ( right.m_suffix.empty() ) return true;
		return m_suffix < right.m_suffix;
	}
	return m_version < right.m_version;
}

} // namespace vdrlive
