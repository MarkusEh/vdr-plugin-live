#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <list>
#include <numeric>
#include <string>
#include "live.h"
#include <vdr/menuitems.h>


namespace vdrlive {

// forward declaration, see below
class cMenuSetupLive;

class Setup
{
	friend Setup& LiveSetup();
	friend class cMenuSetupLive; // friend declaration is not forward 
								 // declaration, although gcc 3.3 claims so

public:
	typedef std::list< std::string > IpList;

	// commandline
	int GetServerPort() const { return m_serverPort; }
	IpList const& GetServerIps() const { return m_serverIps; }
	// vdr-setup
	int GetLastChannel() const { return m_lastChannel == 0 ? std::numeric_limits< int >::max() : m_lastChannel; }
	int GetScreenshotInterval() const { return m_screenshotInterval; }
	std::string GetAdminLogin() const { return m_adminLogin; }
	std::string GetAdminPassword() const { return m_adminPassword; }
	bool UseAuth() const { return m_useAuth; }
	
	void SetLastChannel(int lastChannel) { m_lastChannel = lastChannel; }
	void SetAdminLogin(std::string login) { m_adminLogin = login; }
	void SetAdminPassword(std::string password) { m_adminPassword = password; }
	void SetUseAuth(int auth) { m_useAuth = auth; }
	void SetScrenshotInterval(int interval) { m_screenshotInterval = interval; }

	bool ParseCommandLine( int argc, char* argv[] );
	char const* CommandLineHelp() const;

	bool ParseSetupEntry( char const* name, char const* value );
	
private:
	Setup();
	Setup( Setup const& );

	mutable std::string m_helpString;
	// commandline options
	int m_serverPort;
	IpList m_serverIps;
	// setup options
	int m_lastChannel;
	int m_screenshotInterval;
	
	int m_useAuth;
	std::string m_adminLogin;
	std::string m_adminPassword;

	bool CheckServerPort();
	bool CheckServerIps();
};

Setup& LiveSetup();

class cMenuSetupLive : public cMenuSetupPage {

protected:
  virtual void Store(void);
//  virtual eOSState ProcessKey(eKeys Key);

public:
  cMenuSetupLive();
  
private:
	int m_lastChannel;
	int m_screenshotInterval;
	
	int m_useAuth;
	char m_adminLogin[20];
	char m_adminPassword[20];
};

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
