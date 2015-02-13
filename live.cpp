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
std::string Plugin::m_resourceDirectory;

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
	m_resourceDirectory = canonicalize_file_name(cPlugin::ResourceDirectory( PLUGIN_NAME_I18N ));

	// force status monitor startup
	LiveStatusMonitor();

	// preload files into file Cache
	PreLoadFileCache(m_resourceDirectory);

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
