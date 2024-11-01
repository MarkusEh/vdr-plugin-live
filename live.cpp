/*
 * live.cpp: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 */

#include "live.h"

#include "setup.h"
#include "tools.h"
#include "status.h"
#include "tasks.h"
#include "timers.h"
#include "preload.h"
#include "users.h"
#include "services_live.h"
#include "epgsearch.h"

namespace vdrlive {

const char *Plugin::VERSION        = LIVEVERSION;
const char *Plugin::DESCRIPTION    = LIVESUMMARY;

std::string Plugin::m_configDirectory;
std::string Plugin::m_resourceDirectory;

const std::locale g_locale = std::locale("");
const std::collate<char>& g_collate_char = std::use_facet<std::collate<char> >(g_locale);

cUsers Users;

Plugin::Plugin(void)
{
}

const char *Plugin::CommandLineHelp(void)
{
  return LiveSetup().CommandLineHelp();
}

bool Plugin::ProcessArgs(int argc, char *argv[])
{
  return LiveSetup().ParseCommandLine( argc, argv );
}

bool Plugin::Initialize(void)
{
  m_configDirectory = canonicalize_file_name(cPlugin::ConfigDirectory( PLUGIN_NAME_I18N ));
  m_resourceDirectory = canonicalize_file_name(cPlugin::ResourceDirectory( PLUGIN_NAME_I18N ));

  return LiveSetup().Initialize();
}

bool Plugin::Start(void)
{
  // force status monitor startup
  LiveStatusMonitor();

  // preload files into file Cache
  PreLoadFileCache(m_resourceDirectory);

  // load users
  Users.Load(AddDirectory(m_configDirectory.c_str(), "users.conf"), true);

  // XXX error handling
  m_thread.reset( new ServerThread );
  m_thread->Start();

  m_liveWorker.reset( new cLiveWorker );
  m_liveWorker->Start();

  return true;
}

void Plugin::Stop(void)
{
  m_thread->Stop();
  while (m_liveWorker->Active()) {
    m_liveWorker->Stop();
    sleep(1);
  }
}

void Plugin::Housekeeping(void) {
  SearchResults::CleanQuery();
}

cString Plugin::Active(void)
{
  return NULL;
}

cMenuSetupPage *Plugin::SetupMenu(void)
{
  return new cMenuSetupLive();
}

bool Plugin::SetupParse(const char *Name, const char *Value)
{
  return LiveSetup().ParseSetupEntry( Name, Value );
}

class cLiveImageProviderImp: public cLiveImageProvider {
  public:
    virtual std::string getImageUrl(const std::string &imagePath, bool fullPath = true) {
      if (LiveSetup().GetTvscraperImageDir().empty() || LiveSetup().GetServerUrl().empty()) {
        if (m_errorMessages) {
          if (LiveSetup().GetTvscraperImageDir().empty() )
            esyslog("live: ERROR plugin tvscraper/scraper2vdr missing or to old. tvscraper 1.2.1 or later is required");
          if (LiveSetup().GetServerUrl().empty() )
            esyslog("live: ERROR please provide -u URL,  --url=URL");
        }
        m_errorMessages = false;
        return fullPath?imagePath:LiveSetup().GetTvscraperImageDir() + imagePath;
      }
      return concat(LiveSetup().GetServerUrlImages(), (fullPath?ScraperImagePath2Live(imagePath):imagePath));
    }
    virtual ~cLiveImageProviderImp() {}
  private:
    bool m_errorMessages = true;
};
bool Plugin::Service(const char *Id, void *Data) {
  if (strcmp(Id, "GetLiveImageProvider") == 0) {
    if (Data == NULL) return true;
    cGetLiveImageProvider* call = (cGetLiveImageProvider*) Data;
    call->m_liveImageProvider = std::make_unique<cLiveImageProviderImp>();
    return true;
  }
  return false;
}
} // namespace vdrlive

VDRPLUGINCREATOR(vdrlive::Plugin); // Don't touch this!
