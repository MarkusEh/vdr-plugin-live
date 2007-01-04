#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <list>
#include <numeric>
#include <string>
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
	int GetLastChannel() const { return m_lastChannel == 0 ? std::numeric_limits< int >::max() : m_lastChannel; }
	int GetScreenshotInterval() const { return m_screenshotInterval; }

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
	int m_screenshotInterval;

	bool CheckLibraryPath();
	bool CheckServerPort();
	bool CheckServerIps();
};

inline Setup& LiveSetup()
{
	return LivePlugin().GetLiveSetup();
}

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
