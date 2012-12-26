/*
 * live.cpp: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 */

#include <vdr/config.h>
#include <vdr/plugin.h>
#include "i18n.h"
#include "live.h"
#include "setup.h"
#include "status.h"
#include "tasks.h"
#include "thread.h"
#include "timers.h"
#include "preload.h"
#include "users.h"

namespace vdrlive {

using namespace std;

const char *Plugin::VERSION        = LIVEVERSION;
const char *Plugin::DESCRIPTION    = LIVESUMMARY;

std::string Plugin::m_configDirectory;
#if APIVERSNUM > 10729
std::string Plugin::m_resourceDirectory;
#endif

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

bool Plugin::Start(void)
{
	m_configDirectory = canonicalize_file_name(cPlugin::ConfigDirectory( PLUGIN_NAME_I18N ));
#if APIVERSNUM > 10729
	m_resourceDirectory = canonicalize_file_name(cPlugin::ResourceDirectory( PLUGIN_NAME_I18N ));
#endif

#if VDRVERSNUM < 10507
	RegisterI18n( vdrlive::Phrases );
#endif
	// force status monitor startup
	LiveStatusMonitor();

	// preload files into file Cache
#if APIVERSNUM > 10729
	PreLoadFileCache(m_resourceDirectory);
#else
	PreLoadFileCache(m_configDirectory);
#endif

	// load users
	Users.Load(AddDirectory(m_configDirectory.c_str(), "users.conf"), true);

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
	LiveTaskManager().DoScheduledTasks();
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

} // namespace vdrlive

VDRPLUGINCREATOR(vdrlive::Plugin); // Don't touch this!
