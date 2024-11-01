#ifndef VDR_LIVE_LIVE_H
#define VDR_LIVE_LIVE_H

#include "thread.h"

// STL headers need to be before VDR tools.h (included by <vdr/plugin.h>)
#include <string>

#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#ifndef DISABLE_TEMPLATES_COLLIDING_WITH_STL
// To get rid of the swap definition in vdr/tools.h
#define DISABLE_TEMPLATES_COLLIDING_WITH_STL
#endif
#include <vdr/plugin.h>

namespace vdrlive {

class cLiveWorker;
class Plugin : public cPlugin {
public:
  Plugin(void);
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Start(void);
  virtual bool Initialize(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual cString Active(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);

  static std::string const& GetConfigDirectory() { return m_configDirectory; }
  static std::string const& GetResourceDirectory() { return m_resourceDirectory; }

private:
  static const char *VERSION;
  static const char *DESCRIPTION;

  static std::string m_configDirectory;
  static std::string m_resourceDirectory;

  std::unique_ptr<ServerThread> m_thread;
  std::unique_ptr<cLiveWorker> m_liveWorker;
};

} // namespace vdrlive

#endif // VDR_LIVE_LIVE_H
