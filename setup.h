#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <list>
#include <limits>
#include <numeric>
#include <string>
#include "live.h"
#include "tntfeatures.h"
#include <vdr/menuitems.h>

#define LIVEVERSION "0.3.0"
#define LIVEVERSNUM 300
#define LIVESUMMARY trNOOP("Live Interactive VDR Environment")

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
		int GetServerSslPort() const { return m_serverSslPort; }
		std::string GetServerSslCert() const { return m_serverSslCert; }
		std::string GetServerSslKey() const { return m_serverSslKey; }
		IpList const& GetServerIps() const { return m_serverIps; }
		// vdr-setup
		int GetLastChannel() const { return m_lastChannel == 0 ? std::numeric_limits< int >::max() : m_lastChannel; }
		int GetScreenshotInterval() const { return m_screenshotInterval; }
		std::string const GetAdminLogin() const { return m_adminLogin; }
		std::string const GetMD5HashAdminPassword() const;
		int GetAdminPasswordLength() const;
		bool GetUseAuth() const { return m_useAuth; }
		bool UseAuth() const;
		std::string const GetTimes() const { return m_times; }
		std::string const GetChannelGroups() const { return m_channelGroups; }
		std::string const GetScheduleDuration() const { return m_scheduleDuration; }
		std::string const GetStartScreen() const { return m_startscreen; }
		std::string const GetStartScreenLink() const;
		std::string const GetTheme() const { return m_theme; }
		std::string const GetThemedLink(std::string const & type, const std::string& name) const { return "themes/" + GetTheme() + "/" + type + "/" + name; }
		std::string const GetLocalNetMask() const { return m_localnetmask; };
		bool GetIsLocalNet() const { return m_islocalnet; };
		std::string const GetLastWhatsOnListMode() const { return m_lastwhatsonlistmode; }
		std::string const GetLastSortingMode() const { return m_lastsortingmode; }
		std::string const GetTntnetLogLevel() const { return m_tntnetloglevel; }
		bool GetShowLogo() const { return m_showLogo != 0; }
		bool GetUseAjax() const { return m_useAjax != 0; }
		bool GetShowInfoBox() const { return m_showInfoBox != 0; }
		bool GetUseStreamdev() const { return m_useStreamdev != 0; }
		int GetStreamdevPort() const { return m_streamdevPort; }
		std::string const GetStreamdevType() const { return m_streamdevType; }
		bool GetShowIMDb() const { return m_showIMDb != 0; }
		std::string const GetEpgImageDir() { return m_epgimagedir; }
		bool GetShowChannelsWithoutEPG() const { return m_showChannelsWithoutEPG != 0; }

		void SetLastChannel(int lastChannel) { m_lastChannel = lastChannel; }
		void SetAdminLogin(std::string const & login) { m_adminLogin = login; }
		std::string SetAdminPassword(std::string password);
		void SetUseAuth(int auth) { m_useAuth = auth; }
		void SetScreenshotInterval(int interval) { m_screenshotInterval = interval; }
		void SetTimes(std::string const & times) { m_times = times; }
		void SetChannelGroups(std::string const & channelGroups) { m_channelGroups = channelGroups; }
		void SetScheduleDuration(std::string const & scheduleDuration) { m_scheduleDuration = scheduleDuration; }
		void SetStartScreen(std::string const & startscreen) { m_startscreen = startscreen; }
		void SetTheme(std::string const & theme) { m_theme = theme; }
		void SetLocalNetMask(std::string const & localnetmask) { m_localnetmask = localnetmask; }
		void SetIsLocalNet(bool islocalnet) { m_islocalnet = islocalnet; }
		void SetLastWhatsOnListMode(std::string const & mode) { m_lastwhatsonlistmode = mode; SaveSetup(); }
		void SetLastSortingMode(std::string const & mode) { m_lastsortingmode = mode; SaveSetup(); }
		void SetShowLogo(bool show) { m_showLogo = show ? 1 : 0; }
		void SetUseAjax(bool use) { m_useAjax = use ? 1 : 0; }
		void SetShowInfoBox(bool show) { m_showInfoBox = show ? 1 : 0; }
		void SetUseStreamdev(bool use) { m_useStreamdev = use ? 1 : 0; }
		void SetStreamdevPort(int port) { m_streamdevPort = port; }
		void SetStreamdevType(std::string const & type) { m_streamdevType = type; }
		void SetShowIMDb(bool show) { m_showIMDb = show ? 1 : 0; }
		void SetShowChannelsWithoutEPG(bool show) { m_showChannelsWithoutEPG = show ? 1 : 0; }

		bool SaveSetup();

		bool ParseCommandLine( int argc, char* argv[] );
		char const* CommandLineHelp() const;

		bool ParseSetupEntry( char const* name, char const* value );

		bool CheckLocalNet(std::string const & ip);


	private:
		Setup();
		Setup( Setup const& );

		// me
		cPlugin* liveplugin;

		mutable std::string m_helpString;
		// commandline options
		int m_serverPort;
		int m_serverSslPort;
		std::string m_serverSslCert;
		std::string m_serverSslKey;
		static std::string m_configDirectory;
		IpList m_serverIps;
		std::string m_epgimagedir;

		// setup options
		int m_lastChannel;
		int m_screenshotInterval;

		int m_useAuth;
		std::string m_adminLogin;
		std::string m_adminPasswordMD5;
		std::string m_times;
		std::string m_channelGroups;
		std::string m_scheduleDuration;
		std::string m_startscreen;
		std::string m_theme;
		std::string m_localnetmask;
		bool m_islocalnet;
		std::string m_lastwhatsonlistmode;
		std::string m_lastsortingmode;
		std::string m_tntnetloglevel;
		int m_showLogo;
		int m_useAjax;
		int m_showInfoBox;
		int m_useStreamdev;
		int m_streamdevPort;
		std::string m_streamdevType;
		int m_showIMDb;
		int m_showChannelsWithoutEPG;

		bool CheckServerPort();
		bool CheckServerIps();
		bool CheckServerSslPort();
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
