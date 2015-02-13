#ifndef VDR_LIVE_LIVE_H
#define VDR_LIVE_LIVE_H

#include <memory>
#include <string>
#include <vdr/config.h>
#include <vdr/plugin.h>
#include "thread.h"

namespace vdrlive {

class Plugin : public cPlugin {
public:
	Plugin(void);
	virtual const char *Version(void) { return VERSION; }
	virtual const char *Description(void) { return tr(DESCRIPTION); }
	virtual const char *CommandLineHelp(void);
	virtual bool ProcessArgs(int argc, char *argv[]);
	virtual bool Start(void);
	virtual void Stop(void);
	virtual void MainThreadHook(void);
	virtual cString Active(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *Name, const char *Value);

	static std::string const& GetConfigDirectory() { return m_configDirectory; }
	static std::string const& GetResourceDirectory() { return m_resourceDirectory; }

private:
	static const char *VERSION;
	static const char *DESCRIPTION;

	static std::string m_configDirectory;
	static std::string m_resourceDirectory;

	std::auto_ptr< ServerThread > m_thread;
};

} // namespace vdrlive

#endif // VDR_LIVE_LIVE_H
