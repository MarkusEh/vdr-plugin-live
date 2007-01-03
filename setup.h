#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <string>
#include <list>
#include "live.h"

namespace vdrlive {

class Plugin;

class Setup
{
	friend Setup& Plugin::GetLiveSetup();

public:
	typedef std::list< std::string > IpList;

	// commandline
	std::string const& GetLibraryPath() const { return m_libraryPath; }
	int GetServerPort() const { return m_serverPort; }
	IpList const& GetServerIps() const { return m_serverIps; }
	// vdr-setup
	int GetLastChannel() const { return m_lastChannel; }

	bool ParseCommandLine( int argc, char* argv[] );
	char const* CommandLineHelp() const;

	bool ParseSetupEntry( char const* name, char const* value );
	
private:
	Setup();
	Setup( Setup const& );

	mutable std::string m_helpString;
	// commandline
	std::string m_libraryPath;
	int m_serverPort;
	IpList m_serverIps;
	// vdr-setup
	int m_lastChannel;

	bool CheckLibraryPath();
	bool CheckServerPort();
	bool CheckServerIps();
};

inline Setup& LiveSetup()
{
	static PluginBase& plugin = *static_cast< Plugin* >( cPluginManager::GetPlugin( PLUGIN_NAME_I18N ) );
	return plugin.GetLiveSetup();
}

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
