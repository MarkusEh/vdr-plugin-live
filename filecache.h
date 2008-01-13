#ifndef VDR_LIVE_FILECACHE_H
#define VDR_LIVE_FILECACHE_H

#include <limits>
#include <numeric>
#include <string>
#include <vector>
#include <vdr/thread.h>
#include <vdr/tools.h>
#include "cache.h"

namespace vdrlive {

class FileObject
{
public:
	FileObject( std::string const& path )
		: m_ctime( std::numeric_limits< std::time_t >::max()  )
		, m_path( path ) {}

	std::size_t size() const { return m_data.size(); }
	std::size_t weight() const { return size(); }
	bool is_current() const { return m_ctime == get_filetime( m_path ); }
	bool load();
	char const* data() const { return &m_data[0]; }
	std::time_t ctime() const { return m_ctime; }

private:
	static std::time_t get_filetime( std::string const& path );

	mutable std::time_t m_ctime;
	std::string m_path;
	std::vector< char > m_data;
};

class FileCache: public vgstools::cache< std::string, FileObject >
{
	typedef vgstools::cache< std::string, FileObject > base_type;

public:
	FileCache( size_t maxWeight ): base_type( maxWeight ) {}

	ptr_type get( key_type const& key )
	{
		cMutexLock lock( &m_mutex );
		// dsyslog( "vdrlive::FileCache::get( %s )", key.c_str() );
		// dsyslog( "vdrlive::FileCache had %u entries (weight: %u)", count(), weight() );
		ptr_type result = base_type::get( key );
		// dsyslog( "vdrlive::FileCache now has %u entries (weight: %u)", count(), weight() );
		// dsyslog( "vdrlive::FileCache::get( %s ) = %p", key.c_str(), result.get() );
		return result;
	}

private:
	cMutex m_mutex;
};

//typedef vgstools::cache< std::string, FileObject > FileCache;
FileCache& LiveFileCache();

} // namespace vdrlive

#endif // VDR_LIVE_FILECACHE_H
