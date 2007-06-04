#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <list>
#include <numeric>
#include <string>
#include "live.h"
#include <vdr/menuitems.h>

#define LIVEVERSION "0.1.0"
#define LIVEVERSNUM 100
#define LIVESUMMARY "Live Interactive VDR Environment"

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
	std::string GetMD5HashAdminPassword() const;
	int GetAdminPasswordLength() const;
	bool GetUseAuth() const { return m_useAuth; }
	bool UseAuth() const;
	std::string GetTimes() const { return m_times; }
	std::string GetStartScreen() const { return m_startscreen; }
	std::string GetStartScreenLink() const;
	std::string GetTheme() const { return m_theme; }
	std::string GetThemedLink(const std::string& type, const std::string& name) const { return "themes/" + GetTheme() + "/" + type + "/" + name; }
	std::string GetLocalNetMask() const { return m_localnetmask; };
	bool GetIsLocalNet() const { return m_islocalnet; };
	std::string GetLastWhatsOnListMode() const { return m_lastwhatsonlistmode; }
	std::string GetTntnetLogLevel() const { return m_tntnetloglevel; }
	void SetLastChannel(int lastChannel) { m_lastChannel = lastChannel; }
	void SetAdminLogin(std::string login) { m_adminLogin = login; }
	std::string SetAdminPassword(std::string password);
	void SetUseAuth(int auth) { m_useAuth = auth; }
	void SetScrenshotInterval(int interval) { m_screenshotInterval = interval; }
	void SetTimes(std::string times) { m_times = times; }
	void SetStartScreen(std::string startscreen) { m_startscreen = startscreen; }
	void SetTheme(std::string theme) { m_theme = theme; }
	void SetLocalNetMask(std::string localnetmask) { m_localnetmask = localnetmask; }
	void SetIsLocalNet(bool islocalnet) { m_islocalnet = islocalnet; }

	void SetLastWhatsOnListMode(std::string mode) { m_lastwhatsonlistmode = mode; SaveSetup(); }

	bool SaveSetup();

	bool ParseCommandLine( int argc, char* argv[] );
	char const* CommandLineHelp() const;

	bool ParseSetupEntry( char const* name, char const* value );

	bool HaveEPGSearch(void);
	bool CheckLocalNet(const std::string& ip);

private:
	Setup();
	Setup( Setup const& );

	// me
	cPlugin* liveplugin;

	mutable std::string m_helpString;
	// commandline options
	int m_serverPort;
	IpList m_serverIps;
	// setup options
	int m_lastChannel;
	int m_screenshotInterval;

	int m_useAuth;
	std::string m_adminLogin;
	std::string m_adminPasswordMD5;
	std::string m_times;
	std::string m_startscreen;
	std::string m_theme;
	std::string m_localnetmask;
	bool m_islocalnet;
	std::string m_lastwhatsonlistmode;
	std::string m_tntnetloglevel;

	bool CheckServerPort();
	bool CheckServerIps();
};

Setup& LiveSetup();

class cMenuSetupLive : public cMenuSetupPage {

protected:
	virtual void Store(void);
	virtual eOSState ProcessKey(eKeys Key);
public:
  cMenuSetupLive();

private:
	int m_lastChannel;
	int m_screenshotInterval;

	int m_useAuth;
	char m_adminLogin[20];
	char m_adminPassword[20];
	char m_tmpPassword[20];
	std::string m_oldpasswordMD5;
	std::string m_newpasswordMD5;

	void Set(void);
	bool InEditMode(const char* ItemText, const char* ItemName, const char* ItemValue);
};

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
