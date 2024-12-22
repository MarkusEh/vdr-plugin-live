#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

// STL headers need to be before VDR tools.h (included by <vdr/menuitems.h>)
#include <list>
#include <limits>
#include <string>
#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include "stringhelpers.h"
#include <vdr/menuitems.h>

#define LIVEVERSION "3.3.12"
#define LIVEVERSNUM 30312
#define LIVESUMMARY trNOOP("Live Interactive VDR Environment")

namespace vdrlive {

// directories within /tmp for storing plugin data
#define tmpDir              "/tmp/live/"
#define tmpImageDir         tmpDir "images/"
#define tmpHlsBufferDir     tmpDir "hls-buffer/"

// tags for FFMPEG commands for streaming channels into browser
#define tagChannelH264      "chnH264"
#define tagChannelHVEC      "chnHEVC"
#define tagChannelMPG2      "chnMPG2"
#define tagChannelDFLT      "chnDFLT"
// tags for FFMPEG commands for streaming recordings into browser
#define tagRecordingH264    "recH264"
#define tagRecordingHVEC    "recHVEC"
#define tagRecordingMPG2    "recMPG2"
#define tagRecordingDFLT    "recDFLT"

// forward declaration, see below
class cMenuSetupLive;

class Setup
{
  friend Setup& LiveSetup();
  friend class cMenuSetupLive; // friend declaration is not forward
                 // declaration, although GCC 3.3 claims so

  public:
    typedef std::list<std::string> IpList;

    // command line
    int GetServerPort() const { return m_serverPort; }
    int GetServerSslPort() const { return m_serverSslPort; }
    std::string GetServerSslCert() const { return m_serverSslCert; }
    std::string GetServerSslKey() const { return m_serverSslKey; }
    const std::string &GetServerUrl() const { return m_serverUrl; }
    const std::string &GetServerUrlImages() const { return m_serverUrlImages; }
    IpList const& GetServerIps() const { return m_serverIps; }
    // VDR setup
    int GetLastChannel() const { return m_lastChannel == 0 ? std::numeric_limits<int>::max() : m_lastChannel; }
    int GetScreenshotInterval() const { return m_screenshotInterval; }
    std::string const GetAdminLogin() const { return m_adminLogin; }
    std::string const GetMD5HashAdminPassword() const;
    int GetAdminPasswordLength() const;
    bool GetUseAuth() const { return m_useAuth; }
    bool UseAuth() const;
    std::string const GetTimes() const { return m_times; }
    std::string const GetChannelGroups() const { return m_channelGroups; }
    std::string const GetChannelGroupsGen() const { return m_channelGroupsGen; }
    std::string const GetScheduleDuration() const { return m_scheduleDuration; }
    std::string const GetStartScreen() const { return m_startscreen; }
    std::string const GetStartScreenLink() const;
    std::string const GetTheme() const { return m_theme; }
    std::string const GetThemedLink(std::string const & type, const std::string& name) const { return GetThemedLinkPrefix() + type + "/" + name; }
    std::string const GetThemedLinkPrefix() const { return m_themedLinkPrefix ; }
    std::string const GetThemedLinkPrefixImg() const { return m_themedLinkPrefixImg ; }
    std::string const GetLocalNetMask() const { return m_localnetmask; };
    std::string const GetLocalNetMaskIPv6() const { return m_localnetmaskIPv6; };
    bool GetAllowLocalhost() const { return m_allowlocalhost != 0; };
    bool GetIsLocalNet() const { return m_islocalnet; };
    std::string const GetLastWhatsOnListMode() const { return m_lastwhatsonlistmode; }
    std::string const GetLastSortingMode() const { return m_lastsortingmode; }
    std::string const GetTntnetLogLevel() const { return m_tntnetloglevel; }
    bool GetShowLogo() const { return m_showLogo != 0; }
    bool GetUseArchive() const { return false; }
    bool GetShowInfoBox() const { return m_showInfoBox != 0; }
    bool GetUseStreamdev() const { return m_useStreamdev != 0; }
    int GetStreamdevPort() const { return m_streamdevPort; }
    std::string const GetStreamdevType() const { return m_streamdevType; }
    bool GetMarkNewRec() const { return m_markNewRec != 0; }
    void CheckSetupConfFFmpegConfConsistency(cSv setupCmd, cSv tag);
    std::string ReadStreamVideoFFmpegCmdFromConfigFile(cSv tag) const;
    std::string const GetFFmpegConf() const { return m_ffmpegConf; }
    std::string const GetStreamVideoOpt0() const { return m_streamVopt0; }
    std::string const GetStreamVideoOpt1() const { return m_streamVopt1; }
    std::string const GetStreamVideoOpt2() const { return m_streamVopt2; }
    std::string const GetStreamVideoOpt3() const { return m_streamVopt3; }
    bool GetShowIMDb() const { return m_showIMDb != 0; }
    std::string const GetEpgImageDir() { return m_epgimagedir; }
    const std::string &GetTvscraperImageDir() const { return m_tvscraperimagedir; }
    cPlugin *GetPluginTvscraper() { return m_p_tvscraper; } // tvscraper
    cPlugin *GetPluginScraper() { return m_p_scraper; } // tvscraper. Or, if not available, scraper2vdr
    void SetTvscraperImageDir(const std::string &dir);
    const std::string &GetChanLogoDir() const { return m_chanlogodir; }
    bool GetShowChannelsWithoutEPG() const { return m_showChannelsWithoutEPG != 0; }
    int GetMaxTooltipChars() const { return m_maxTooltipChars; }

    void SetLastChannel(int lastChannel) { m_lastChannel = lastChannel; }
    void SetAdminLogin(std::string const & login) { m_adminLogin = login; }
    std::string SetAdminPassword(std::string password);
    void SetUseAuth(int auth) { m_useAuth = auth; }
    void SetScreenshotInterval(int interval) { m_screenshotInterval = interval; }
    void SetTimes(std::string const & times) { m_times = times; }
    void SetChannelGroups(std::string const & channelGroups) { m_channelGroups = channelGroups; }
    void SetChannelGroupsGen(std::string const & channelGroupsGen) { m_channelGroupsGen = channelGroupsGen; }
    void SetScheduleDuration(std::string const & scheduleDuration) { m_scheduleDuration = scheduleDuration; }
    void SetStartScreen(std::string const & startscreen) { m_startscreen = startscreen; }
    void SetTheme(std::string const & theme) { m_theme = theme; m_themedLinkPrefix = "themes/" + theme + "/"; m_themedLinkPrefixImg = m_themedLinkPrefix + "img/"; }
    void SetLocalNetMask(std::string const & localnetmask) { m_localnetmask = localnetmask; }
    void SetLocalNetMaskIPv6(std::string const & localnetmaskIPv6) { m_localnetmaskIPv6 = localnetmaskIPv6; }
    void SetAllowLocalhost(bool allow) { m_allowlocalhost = allow ? 1 : 0; }
    void SetIsLocalNet(bool islocalnet) { m_islocalnet = islocalnet; }
    void SetLastWhatsOnListMode(std::string const & mode) { m_lastwhatsonlistmode = mode; SaveSetup(); }
    void SetLastSortingMode(std::string const & mode) { m_lastsortingmode = mode; SaveSetup(); }
    void SetShowLogo(bool show) { m_showLogo = show ? 1 : 0; }
    void SetShowInfoBox(bool show) { m_showInfoBox = show ? 1 : 0; }
    void SetUseStreamdev(bool use) { m_useStreamdev = use ? 1 : 0; }
    void SetStreamdevPort(int port) { m_streamdevPort = port; }
    void SetStreamdevType(std::string const & type) { m_streamdevType = type; }
    void SetMarkNewRec(bool show) { m_markNewRec = show ? 1 : 0; }
    void SetShowIMDb(bool show) { m_showIMDb = show ? 1 : 0; }
    void SetShowChannelsWithoutEPG(bool show) { m_showChannelsWithoutEPG = show ? 1 : 0; }

    bool SaveSetup();

    bool ParseCommandLine( int argc, char* argv[] );
    bool Initialize( void );
    char const* CommandLineHelp() const;

    bool ParseSetupEntry( char const* name, char const* value );

    bool CheckLocalNet(std::string const & ip);
    time_t GetVdrStart() { return m_vdr_start; }


  private:
    Setup();
    Setup( Setup const& );

    // me
    cPlugin* liveplugin;

    mutable std::string m_helpString;
    // command-line options
    int m_serverPort;
    int m_serverSslPort;
    std::string m_serverSslCert;
    std::string m_serverSslKey;
    std::string m_serverUrl;
    std::string m_serverUrlImages;
    static std::string m_configDirectory;
    IpList m_serverIps;
    std::string m_epgimagedir;
    std::string m_tvscraperimagedir;
                cPlugin *m_p_tvscraper;
                cPlugin *m_p_scraper;
    std::string m_chanlogodir;

    // setup options
    int m_lastChannel;
    int m_screenshotInterval;

    int m_useAuth;
    std::string m_adminLogin;
    std::string m_adminPasswordMD5;
    std::string m_times;
    std::string m_channelGroupsGen;
    std::string m_channelGroups;
    std::string m_scheduleDuration;
    std::string m_startscreen;
    std::string m_theme;
    std::string m_themedLinkPrefix;
    std::string m_themedLinkPrefixImg;
    std::string m_localnetmask;
    std::string m_localnetmaskIPv6;
    int m_allowlocalhost;
    bool m_islocalnet;
    std::string m_lastwhatsonlistmode;
    std::string m_lastsortingmode;
    std::string m_tntnetloglevel;
    int m_showLogo;
    int m_showInfoBox;
    int m_useStreamdev;
    int m_streamdevPort;
    std::string m_streamdevType;
    int m_markNewRec;
    std::string m_ffmpegConf;
    std::string m_streamVopt0;
    std::string m_streamVopt1;
    std::string m_streamVopt2;
    std::string m_streamVopt3;
    int m_showIMDb;
    int m_showChannelsWithoutEPG;

    const int m_maxTooltipChars = 300; // maximum number of characters to be displayed in tooltips
    bool CheckServerPort();
    bool CheckServerIps();
    bool CheckServerSslPort();
    time_t m_vdr_start = 0;
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
