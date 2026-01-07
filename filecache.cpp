/*
#include "filecache.h"

#include <algorithm>
#include <fstream>
#include <istream>
#include <sys/stat.h>

namespace vdrlive {

std::time_t FileObject::get_filetime(cStr path)
{
  struct stat sbuf;
  if ( stat( path.c_str(), &sbuf ) < 0 ) {
// errno == 2 == ENOENT No such file or directory
// so this is also used to detect if a file exists
    if (errno != 2) esyslog3("file ", path, " not found, errno =", errno);
    return 0;
  }
  return sbuf.st_ctime;
}

bool FileObject::load()
{
  m_file.load(m_path);
  m_ctime = get_filetime(m_path);
  return m_file.exists();
}

FileCache& LiveFileCache()
{
//  static FileCache instance( 1000000 );
  static FileCache instance( 1 );
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
*/
