
#include "livefeatures.h"

#include "tools.h"

namespace vdrlive {

SplitVersion::SplitVersion(cSv version)
  : m_version(0)
{
  static const int factors[] = { 100000000, 1000000, 1000, 1 };

  size_t pos = version.find('-');
  if (pos != std::string_view::npos) {
    m_suffix = version.substr(pos + 1);
    version.remove_suffix(version.size() - pos);
  }
  size_t i = 0;
  for (int part: cSplit<int>(version, '.' )) {
    if (i >= sizeof(factors)/sizeof(factors[0])) break;
    m_version += part * factors[i];
    ++i;
  }
}

bool SplitVersion::operator<( const SplitVersion& right ) const
{
  if ( m_version == right.m_version ) {
    if (right.m_suffix.empty()) {
      return false;
    }
    return m_suffix < right.m_suffix;
  }
  return m_version < right.m_version;
}

} // namespace vdrlive
