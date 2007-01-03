/*
 * httpd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: live.cpp,v 1.4 2007/01/03 15:54:31 lordjaxom Exp $
 */

#include <memory>
#include <vdr/plugin.h>
#include "i18n.h"
#include "setup.h"
#include "thread.h"

namespace vdrlive {

using namespace std;

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Live Integrated VDR Environment";

class Plugin : public cPlugin {
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

private:
	auto_ptr< ServerThread > m_thread;
};

Plugin::Plugin(void)
{
}

const char *Plugin::CommandLineHelp(void)
{
	return Setup::Get().Help();
}

bool Plugin::ProcessArgs(int argc, char *argv[])
{
	return Setup::Get().Parse( argc, argv );
}

bool Plugin::Start(void)
{
	RegisterI18n( vdrlive::Phrases );
	// XXX error handling
	m_thread.reset( new ServerThread );
	m_thread->Start();
	return true;
}

void Plugin::Stop(void)
{
}

void Plugin::MainThreadHook(void)
{
}

cString Plugin::Active(void)
{
	return NULL;
}

cMenuSetupPage *Plugin::SetupMenu(void)
{
	return NULL;
}

bool Plugin::SetupParse(const char *Name, const char *Value)
{
	return true;
}

} // namespace vdrlive

VDRPLUGINCREATOR(vdrlive::Plugin); // Don't touch this!
