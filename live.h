#ifndef VDR_LIVE_LIVE_H
#define VDR_LIVE_LIVE_H

#include <memory>
#include <vdr/plugin.h>
#include "thread.h"

namespace vdrlive {

class Setup;

class PluginBase : public cPlugin 
{
public:
	virtual Setup& GetLiveSetup() = 0;
};

class Plugin : public PluginBase {
public:
	Plugin(void);
	virtual const char *Version(void) { return VERSION; }
	virtual const char *Description(void) { return DESCRIPTION; }
	virtual const char *CommandLineHelp(void);
	virtual bool ProcessArgs(int argc, char *argv[]);
	virtual bool Start(void);
	virtual void Stop(void);
	virtual void MainThreadHook(void);
	virtual cString Active(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *Name, const char *Value);

	virtual Setup& GetLiveSetup();

private:
	static const char *VERSION;
	static const char *DESCRIPTION;

	std::auto_ptr< ServerThread > m_thread;
};

} // namespace vdrlive

#endif // VDR_LIVE_LIVE_H
