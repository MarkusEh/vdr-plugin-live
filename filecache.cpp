#include <algorithm>
#include <fstream>
#include <istream>
#include <sys/stat.h>
#include "filecache.h"

namespace vdrlive {

std::time_t FileObject::get_filetime( std::string const& path )
{
	struct stat sbuf;
	if ( stat( path.c_str(), &sbuf ) < 0 )
		return 0;
	return sbuf.st_ctime;
}

bool FileObject::load()
{
	std::ifstream ifs( m_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
	if ( !ifs )
		return false;

	std::streamsize size = ifs.tellg();
	ifs.seekg( 0, std::ios::beg );

	std::vector< char > data( size );
	data.resize( size );
	ifs.read( &data[0], size );
	ifs.close();

	m_ctime = get_filetime( m_path );
	m_data.swap( data );
	return true;
}

FileCache& LiveFileCache()
{
	static FileCache instance( 1000000 );
	return instance;
}

} // namespace vdrlive

#if 0
using namespace vdrlive;

int main()
{
	FileCache::ptr_type f = LiveFileCache().get("/tmp/live/active.png");
}
#endif
