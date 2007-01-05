/*
 * httpd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: live.cpp,v 1.9 2007/01/05 17:52:46 lordjaxom Exp $
 */

#include <vdr/plugin.h>
#include "i18n.h"
#include "live.h"
#include "setup.h"
#include "thread.h"
#include "timers.h"

namespace vdrlive {

using namespace std;

const char *Plugin::VERSION        = "0.0.1";
const char *Plugin::DESCRIPTION    = "Live Integrated VDR Environment";

std::string Plugin::m_configDirectory;

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

bool Plugin::Start(void)
{
	m_configDirectory = cPlugin::ConfigDirectory( PLUGIN_NAME_I18N );

	RegisterI18n( vdrlive::Phrases );
	// XXX error handling
	m_thread.reset( new ServerThread );
	m_thread->Start();
	return true;
}

void Plugin::Stop(void)
{
	m_thread->Stop();
}

void Plugin::MainThreadHook(void)
{
	LiveTimerManager().DoPendingWork();
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
	return LiveSetup().ParseSetupEntry( Name, Value );
}

Setup& Plugin::GetLiveSetup()
{
	static Setup instance;
	return instance;
}

TimerManager& Plugin::GetLiveTimerManager()
{
	static TimerManager instance;
	return instance;
}

} // namespace vdrlive

VDRPLUGINCREATOR(vdrlive::Plugin); // Don't touch this!
