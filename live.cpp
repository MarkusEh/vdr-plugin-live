/*
 * httpd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: live.cpp,v 1.5 2007/01/03 21:43:21 lordjaxom Exp $
 */

#include <vdr/plugin.h>
#include "i18n.h"
#include "live.h"
#include "setup.h"
#include "thread.h"

namespace vdrlive {

using namespace std;

const char *Plugin::VERSION        = "0.0.1";
const char *Plugin::DESCRIPTION    = "Live Integrated VDR Environment";

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
	return LiveSetup().ParseSetupEntry( Name, Value );
}

Setup& Plugin::GetLiveSetup()
{
	static Setup instance;
	return instance;
}

} // namespace vdrlive

VDRPLUGINCREATOR(vdrlive::Plugin); // Don't touch this!
