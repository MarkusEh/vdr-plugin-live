#ifndef VDR_LIVE_FEATURES_H
#define VDR_LIVE_FEATURES_H

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <string>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/plugin.h>
#include "stringhelpers.h"

namespace vdrlive {

class SplitVersion
{
public:
  explicit SplitVersion(cSv version);
  bool operator<(const SplitVersion& right) const;

private:
  int m_version;
  std::string m_suffix;
};

template<typename Feat>
class Features;

template<typename Feat>
Features<Feat>& LiveFeatures();

template<typename Feat>
class Features
{
  friend Features<Feat>& LiveFeatures<>();

public:
  bool Loaded() const { return m_plugin != 0; }
  bool Recent() const { return !(m_version < m_minVersion); }
  char const* Version() const { return m_plugin ? m_plugin->Version() : ""; }
  char const* MinVersion() const { return Feat::MinVersion(); }

private:
  cPlugin* m_plugin;
  SplitVersion m_version;
  SplitVersion m_minVersion;

  Features()
    : m_plugin( cPluginManager::GetPlugin( Feat::Plugin() ) )
    , m_version( Version() )
    , m_minVersion( Feat::MinVersion() ) {}
};

template<typename Feat>
Features<Feat>& LiveFeatures()
{
  static Features<Feat> instance;
  return instance;
}

namespace features
{
  struct epgsearch
  {
    static const char* Plugin() { return "epgsearch"; }
    static const char* MinVersion() { return "0.9.25.beta6"; }
  };

  struct streamdev_server
  {
    static const char* Plugin() { return "streamdev-server"; }
    static const char* MinVersion() { return "0.6.0"; }
  };

  struct tvscraper
  {
    static const char* Plugin() { return "tvscraper"; }
    static const char* MinVersion() { return "1.2.13"; }
  };
} // namespace features

} // namespace vdrlive

#endif // VDR_LIVE_FEATURES_H
