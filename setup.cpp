#include "live.h"
#include "setup.h"
#include "services.h"

#include "tools.h"
#include "tntfeatures.h"
#include "stringhelpers.h"

// STL headers need to be before VDR tools.h (included by <vdr/plugin.h>)
#include <getopt.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <bitset>

#include <vdr/plugin.h>

#ifdef __FreeBSD__
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <arpa/inet.h>

namespace vdrlive {

Setup::Setup():
    m_serverPort( 8008 ),
    m_serverSslPort( 8443 ),
    m_serverSslCert(),
    m_serverSslKey(),
    m_lastChannel( 0 ),
    m_screenshotInterval( 1000 ),
    m_useAuth( 1 ),
    m_adminLogin("admin"),
    m_channelGroupsGen(""),
    m_channelGroups(""),
    m_scheduleDuration( "8" ),
    m_theme("marine"),
    m_themedLinkPrefix("themes/" + m_theme + "/"),
    m_themedLinkPrefixImg("themes/marine/img/"),
    m_allowlocalhost(0),
    m_lastwhatsonlistmode("detail"),
    m_lastsortingmode("nameasc"),
    m_tntnetloglevel("WARN"),
    m_showLogo(1),
    m_showInfoBox(1),
    m_useStreamdev(1),
    m_streamdevPort(3000),
    m_streamdevType("TS"),
    m_markNewRec(1),

    m_showIMDb(1),
    m_showChannelsWithoutEPG(0)
{
  m_adminPasswordMD5 = "4:" + MD5Hash("live");
  liveplugin = cPluginManager::GetPlugin("live");
  m_vdr_start = time(0);
}

bool Setup::ParseCommandLine( int argc, char* argv[] )
{
  static struct option opts[] = {
      { "url", required_argument, NULL, 'u' },
      { "port", required_argument, NULL, 'p' },
      { "ip",   required_argument, NULL, 'i' },
      { "log",  required_argument, NULL, 'l' },
      { "epgimages",  required_argument, NULL, 'e' },
      { "sslport", required_argument, NULL, 's' },
      { "cert", required_argument, NULL, 'c' },
      { "key", required_argument, NULL, 'k' },
      { "chanlogos",  required_argument, NULL, '1' },
      { 0 }
  };

  m_serverUrl = "";
  int optchar, optind = 0;
  while ( ( optchar = getopt_long( argc, argv, "p:i:l:e:s:c:", opts, &optind ) ) != -1 ) {
    switch ( optchar ) {
    case 'u': m_serverUrl = optarg; break;
    case 'p': m_serverPort = atoi( optarg ); break;
    case 'i': m_serverIps.push_back( optarg ); break;
    case 'l': m_tntnetloglevel = optarg; break;
    case 'e': m_epgimagedir = optarg; break;
    case 's': m_serverSslPort = atoi( optarg ); break;
    case 'c': m_serverSslCert = optarg; break;
    case 'k': m_serverSslKey = optarg; break;
    case '1': m_chanlogodir = optarg;
      if(!m_chanlogodir.empty() && m_chanlogodir[m_chanlogodir.length()-1] != '/') m_chanlogodir += "/";
      break;
    default:  return false;
    }
  }
  if (!m_serverUrl.empty() ) {
    m_serverUrlImages = m_serverUrl + ":";
    m_serverUrlImages += cSv(cToSvInt(m_serverPort));
    m_serverUrlImages += "/tvscraper/";
  } else m_serverUrlImages = "";

  return CheckServerPort() &&
       CheckServerSslPort() &&
       CheckServerIps();
}

bool Setup::Initialize( void )
{
  cToSvConcat cmdBuff("mkdir -p ", tmpDir, " && mkdir -p ", tmpImageDir);
  int s = system(cmdBuff.c_str());
  if (s < 0)
    esyslog("live: ERROR: Couldn't execute command %s", cmdBuff.c_str() );
  m_p_tvscraper = cPluginManager::GetPlugin("tvscraper");
  if (m_p_tvscraper)
    m_p_scraper = m_p_tvscraper;
  else
    m_p_scraper = cPluginManager::GetPlugin("scraper2vdr");

  // set a default channel-logo directory
  if (!m_chanlogodir.empty()) {
      if (m_chanlogodir[0] != '/' )
        m_chanlogodir = Plugin::GetConfigDirectory() + "/" + m_chanlogodir;
  } else
    m_chanlogodir = Plugin::GetConfigDirectory() + "/logos/";

  // check whether FFMPEG configuration file already exists
  m_ffmpegConf = Plugin::GetConfigDirectory() + "/ffmpeg.conf";
  bool haveFFmpegConf = false;
  try {
    std::ifstream config(m_ffmpegConf);
    haveFFmpegConf = config.good();
    config.close();
  }
  catch (std::exception const& ex) {
    dsyslog("Live: cannot access ffmpeg.conf: \"%s\": %s", m_ffmpegConf.c_str(), ex.what());
  }
  if (haveFFmpegConf) {
    esyslog("live: use ffmpeg.conf file %s",  m_ffmpegConf.c_str() );
    Setup::CheckSetupConfFFmpegConfConsistency(m_streamVopt0, tagChannelH264);
    Setup::CheckSetupConfFFmpegConfConsistency(m_streamVopt1, tagChannelHVEC);
    Setup::CheckSetupConfFFmpegConfConsistency(m_streamVopt2, tagChannelMPG2);
    Setup::CheckSetupConfFFmpegConfConsistency(m_streamVopt3, tagChannelDFLT);
    }
  else
    esyslog("live: ffmpeg.conf file %s does not exist",  m_ffmpegConf.c_str() );
  return true;
}
std::string Setup::ReadStreamVideoFFmpegCmdFromConfigFile(cSv tag) const {
  // read command for tag from FFMPG configuration file
  std::string line;
  std::string packerCmd;
  static const char *ws = "\t ";          // whitespace separators between tags and commands
  try {
    std::ifstream config(LiveSetup().GetFFmpegConf());
    while (std::getline(config, line)) {
      size_t tag_pos = line.find_first_not_of(ws);
      if (tag_pos == std::string::npos) continue;                             // empty
      if (line[tag_pos] == '#') continue;                                     // comment
      if (line.compare(tag_pos, tag.length(), tag)) continue;     // wrong tag prefix
      size_t sep = line.find_first_of(ws, tag_pos + tag.length());
      if (sep == std::string::npos) continue;                             // unterminated tag
      size_t cmd = line.find_first_not_of(ws, sep);
      if (cmd == std::string::npos) continue;                              // no command
      packerCmd = line.substr(cmd);
      break;
    }
    config.close();
  }
  catch (std::exception const& ex) {
    esyslog("ERROR: live reading file \"%s\": %s", LiveSetup().GetFFmpegConf().c_str(), ex.what());
  }
  return packerCmd;
}
void Setup::CheckSetupConfFFmpegConfConsistency(cSv setupCmd, cSv tag) {
// compare entry in ffmpeg.conf with entry in setup.conf, and report error if they differ
  if (setupCmd.empty()) return;
  std::string cmdFFmpegConf = ReadStreamVideoFFmpegCmdFromConfigFile(tag);
  if (cmdFFmpegConf.empty()) {
    esyslog("live: ERROR: tag %.*s missing in ffmpeg.conf", (int)tag.length(), tag.data());
    return;
  }
  if (cmdFFmpegConf != setupCmd) {
    esyslog("live: ERROR: tag %.*s, values differ between ffmpeg.conf and setup.conf. The values in ffmpeg.conf will be used. Please ensure that the values in ffmpeg.conf are correct, and delete the live.StreamVideoOpt* entries in setup.conf", (int)tag.length(), tag.data());
    esyslog("live: cmdFFmpegConf \"%s\"", cmdFFmpegConf.c_str());
    esyslog("live: setupCmd      \"%.*s\"", (int)setupCmd.length(), setupCmd.data());
  }
}

char const* Setup::CommandLineHelp() const
{
  if ( m_helpString.empty() ) {
    std::stringstream builder;
    builder << "  -p PORT,   --port=PORT              use PORT to listen for incoming connections\n"
           "                                      (default: " << m_serverPort << ")\n"
        << "  -i IP,     --ip=IP                  bind server only to specified IP, may appear\n"
           "                                      multiple times\n"
           "                                      (default: 0.0.0.0)\n"
        << "  -s PORT,   --sslport=PORT           use PORT to listen for incoming SSL connections\n"
           "                                      (default: " << m_serverSslPort << ")\n"
        << "  -u URL,    --url=URL                URL to this live server, e.g. http://rpi.fritz.box\n"
           "                                      only required if live is used as image server, e.g. for vnsi\n"
        << "  -c CERT,   --cert=CERT              full path to a custom SSL certificate file\n"
        << "  -k KEY,    --key=KEY                full path to a custom SSL certificate key file\n"
        << "  -l level,  --log=level              log level for Tntnet (values: WARN, ERROR, INFO, DEBUG, TRACE)\n"
        << "  -e <dir>,  --epgimages=<dir>        directory for EPG images\n"
        << "             --chanlogos=<dir>        directory for channel logos (PNG)\n";
    m_helpString = builder.str();
  }
  return m_helpString.c_str();
}

bool Setup::ParseSetupEntry( char const* name, char const* value )
{
  if ( strcmp( name, "LastChannel" ) == 0 ) m_lastChannel = atoi( value );
  else if ( strcmp( name, "ScreenshotInterval" ) == 0 ) m_screenshotInterval = atoi( value );
  else if ( strcmp( name, "UseAuth" ) == 0 ) m_useAuth = atoi( value );
  else if ( strcmp( name, "AdminLogin" ) == 0 ) m_adminLogin = value;
  else if ( strcmp( name, "AdminPasswordMD5" ) == 0 ) m_adminPasswordMD5 = value;
  else if ( strcmp( name, "UserdefTimes" ) == 0 ) m_times = value;
  else if ( strcmp( name, "ChannelGroupsGen" ) == 0 ) m_channelGroupsGen = value;
  else if ( strcmp( name, "ChannelGroups" ) == 0 ) { m_channelGroups = value;
     if (!m_channelGroups.empty() && m_channelGroupsGen.empty()) m_channelGroupsGen = "individual"; } // default if ChannelGroups are available
  else if ( strcmp( name, "ScheduleDuration" ) == 0 ) m_scheduleDuration = value;
  else if ( strcmp( name, "StartPage" ) == 0 ) m_startscreen = value;
  else if ( strcmp( name, "Theme" ) == 0 ) SetTheme(value);
  else if ( strcmp( name, "LocalNetMask" ) == 0 ) { m_localnetmask = value; }
  else if ( strcmp( name, "LocalNetMaskIPv6" ) == 0 ) { m_localnetmaskIPv6 = value; }
  else if ( strcmp( name, "AllowLocalhost" ) == 0 ) { m_allowlocalhost = atoi(value); }
  else if ( strcmp( name, "LastWhatsOnListMode" ) == 0 ) { m_lastwhatsonlistmode = value; }
  else if ( strcmp( name, "LastSortingMode" ) == 0 ) { m_lastsortingmode = value; }
  else if ( strcmp( name, "ShowLogo" ) == 0 ) { m_showLogo = atoi(value); }
  else if ( strcmp( name, "ShowInfoBox" ) == 0 ) { m_showInfoBox = atoi(value); }
  else if ( strcmp( name, "UseStreamdev" ) == 0 ) { m_useStreamdev = atoi(value); }
  else if ( strcmp( name, "StreamdevPort" ) == 0 ) { m_streamdevPort = atoi(value); }
  else if ( strcmp( name, "StreamdevType" ) == 0 ) { m_streamdevType = value; }
  else if ( strcmp( name, "StreamVideoOpt0" ) == 0 ) { m_streamVopt0 = value; }
  else if ( strcmp( name, "StreamVideoOpt1" ) == 0 ) { m_streamVopt1 = value; }
  else if ( strcmp( name, "StreamVideoOpt2" ) == 0 ) { m_streamVopt2 = value; }
  else if ( strcmp( name, "StreamVideoOpt3" ) == 0 ) { m_streamVopt3 = value; }
  else if ( strcmp( name, "ScreenShotInterval" ) == 0 ) { m_screenshotInterval = atoi(value); }
  else if ( strcmp( name, "MarkNewRec" ) == 0 ) { m_markNewRec = atoi(value); }
  else if ( strcmp( name, "ShowIMDb" ) == 0 ) { m_showIMDb = atoi(value); }
  else if ( strcmp( name, "ShowChannelsWithoutEPG" ) == 0 ) { m_showChannelsWithoutEPG = atoi(value); }
  else return false;
  return true;
}

bool Setup::CheckServerPort()
{
  if ( m_serverPort <= 0 || m_serverPort > std::numeric_limits<uint16_t>::max() ) {
    esyslog( "live: ERROR: server port %d is not a valid port number", m_serverPort );
    std::cerr << "ERROR: live server port " << m_serverPort << " is not a valid port number" << std::endl;
    return false;
  }
  return true;
}

bool Setup::CheckServerSslPort()
{
  if ( m_serverSslPort < 0 || m_serverSslPort > std::numeric_limits<uint16_t>::max() ) {
    esyslog( "live: ERROR: server SSL port %d is not a valid port number", m_serverSslPort );
    std::cerr << "ERROR: live server SSL port " << m_serverSslPort << " is not a valid port number" << std::endl;
    return false;
  }
  return true;
}

namespace {
  struct IpValidator
  {
    bool operator() (std::string const & ip)
    {
      struct in6_addr buf;
      struct in_addr buf4;

      esyslog( "live: INFO: validating server IP '%s'", ip.c_str());
      std::cerr << "INFO: validating live server IP '" << ip << "'" << std::endl;
      bool valid = inet_aton(ip.c_str(), &buf4) || inet_pton(AF_INET6, ip.c_str(), &buf);

      if (!valid) {
        esyslog( "live: ERROR: server IP %s is not a valid IP address", ip.c_str());
        std::cerr << "ERROR: live server IP '" << ip << "' is not a valid IP address" << std::endl;
      }
      return valid;
    }
  };
}

bool Setup::CheckServerIps()
{
  if ( m_serverIps.empty() ) {
#if TNT_IPV6_V6ONLY
      m_serverIps.push_back("");
      return true;
#else
    FILE* f = fopen("/proc/sys/net/ipv6/bindv6only", "r");
    if (f) {
      bool bindv6only = false;
      int c = fgetc(f);
      if (c != EOF) {
        bindv6only = ((c - '0') != 0);
      }
      fclose(f);
      f = NULL;
      esyslog( "live: INFO: bindv6only=%d", bindv6only);
      // add a default IPv6 listener address
      m_serverIps.push_back("::");
      // skip the default IPv4 listener address if the IPv6 one will be bound also to v4
      if (!bindv6only)
        return true;
    }
    // add a default IPv4 listener address
    m_serverIps.push_back("0.0.0.0");
    // we assume these are OK :)
    return true;
#endif // TNT_IPV6_V6ONLY
  }

  IpList::iterator i = partition(m_serverIps.begin(), m_serverIps.end(), IpValidator());
  m_serverIps.erase(i, m_serverIps.end());

  return !m_serverIps.empty();
}

std::string const Setup::GetMD5HashAdminPassword() const
{
  // format is <length>:<md5-hash of password>
  std::vector<std::string> parts = StringSplit( m_adminPasswordMD5, ':' );
  return (parts.size() > 1) ? parts[1] : "";
}

int Setup::GetAdminPasswordLength() const
{
  // format is <length>:<md5-hash of password>
  std::vector<std::string> parts = StringSplit( m_adminPasswordMD5, ':' );
  return (parts.size() > 0) ? parse_int<int>( parts[0] ) : 0;
}

std::string Setup::SetAdminPassword(std::string password)
{
  std::stringstream passwordStr;
  passwordStr << password.size() << ":" << MD5Hash(password);
  m_adminPasswordMD5 = passwordStr.str();
  return m_adminPasswordMD5;
}

std::string const Setup::GetStartScreenLink() const
{
  if (m_startscreen == "whatsonnext")
    return "whats_on.html?type=next";
  else if (m_startscreen == "schedule")
    return "schedule.html";
  else if (m_startscreen == "multischedule")
    return "multischedule.html";
  else if (m_startscreen == "timers")
    return "timers.html";
  else if (m_startscreen == "recordings")
    return "recordings.html";
  else
    return "whats_on.html?type=now";
}

bool Setup::UseAuth() const
{
  return m_useAuth && !GetIsLocalNet();
}

void Setup::SetTvscraperImageDir(const std::string &dir) {
  if (dir.empty()) {
    m_tvscraperimagedir = dir;
    return;
  }
  if (dir[dir.length()-1] != '/') m_tvscraperimagedir = dir + "/";
  else m_tvscraperimagedir = dir;
}

bool CheckIpAddress(const std::string& ip, std::string allowedIPv4, std::string allowedIPv6)
{
  // split IPv4 local net mask in net and range
  std::vector<std::string> parts = StringSplit( allowedIPv4, '/' );
  if (parts.size() == 2 && parts[0].find_first_not_of("0123456789./") == std::string::npos && parts[1].find_first_not_of("0123456789") == std::string::npos) {
    std::string net = parts[0];
    int range = parse_int<int>(parts[1]);
    // split IPv6 net and address into the 4 subcomponents
    std::vector<std::string> netparts = StringSplit( net, '.' );
    std::vector<std::string> addrparts = StringSplit( ip, '.' );
    if (netparts.size() == 4 & addrparts.size() == 4) {
      // IPv4 to binary representation
      std::stringstream bin_netstream;
      bin_netstream
        << std::bitset<8>(parse_int<long>(netparts[0]))
        << std::bitset<8>(parse_int<long>(netparts[1]))
        << std::bitset<8>(parse_int<long>(netparts[2]))
        << std::bitset<8>(parse_int<long>(netparts[3]));
      std::stringstream bin_addrstream;
      bin_addrstream
        << std::bitset<8>(parse_int<long>(addrparts[0]))
        << std::bitset<8>(parse_int<long>(addrparts[1]))
        << std::bitset<8>(parse_int<long>(addrparts[2]))
        << std::bitset<8>(parse_int<long>(addrparts[3]));
      // compare range
      std::string bin_net = bin_netstream.str();
      std::string bin_addr = bin_addrstream.str();
      std::string bin_net_range(bin_net.begin(), bin_net.begin() + range);
      std::string addr_net_range(bin_addr.begin(), bin_addr.begin() + range);
      return (bin_net_range == addr_net_range);
    }
  }
  // split IPv6 local net mask into net and range
  // TODO: correctly decode IPv6 addresses with arbitrary abbreviation
  parts = StringSplit( allowedIPv6, '/' );
  if (parts.size() == 2 && parts[0].find_first_not_of("0123456789AaBbCcDdEeFf:") == std::string::npos && parts[1].find_first_not_of("0123456789") == std::string::npos) {
    std::string net = parts[0];
    int range = parse_int<int>(parts[1]);
    // split IPv6 net and IP address into the 8 subcomponents; as the TNT library
    // erroneously reports IPv6 addresses with an abbreviation at the start, which
    // would intersect with the IETF reserved prefix of 0000::/3, we ignore it
    size_t addrstart = ip.find_first_not_of(':');
    if (addrstart == std::string::npos) return false;
    std::vector<std::string> netparts = StringSplit( net, ':' );
    std::vector<std::string> addrparts = StringSplit( ip.substr(addrstart), ':' );
    if (3 <= netparts.size() && netparts.size() <= 8 && 1 <= addrparts.size() && addrparts.size() <= 8) {
      // IPv6 to binary representation; for simplification, we assume that IPv6 addresses
      // have an abbreviation only at the end
      std::stringstream bin_netstream;
      bin_netstream << std::bitset<16>(netparts.size() > 0 ? parse_hex<long>(netparts[0]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 1 ? parse_hex<long>(netparts[1]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 2 ? parse_hex<long>(netparts[2]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 3 ? parse_hex<long>(netparts[3]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 4 ? parse_hex<long>(netparts[4]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 5 ? parse_hex<long>(netparts[5]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 6 ? parse_hex<long>(netparts[6]) : 0L);
      bin_netstream << std::bitset<16>(netparts.size() > 7 ? parse_hex<long>(netparts[7]) : 0L);
      std::stringstream bin_addrstream;
      bin_addrstream << std::bitset<16>(addrparts.size() > 0 ? parse_hex<long>(addrparts[0]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 1 ? parse_hex<long>(addrparts[1]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 2 ? parse_hex<long>(addrparts[2]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 3 ? parse_hex<long>(addrparts[3]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 4 ? parse_hex<long>(addrparts[4]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 5 ? parse_hex<long>(addrparts[5]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 6 ? parse_hex<long>(addrparts[6]) : 0L);
      bin_addrstream << std::bitset<16>(addrparts.size() > 7 ? parse_hex<long>(addrparts[7]) : 0L);
      // compare range
      std::string bin_net = bin_netstream.str();
      std::string bin_addr = bin_addrstream.str();
      std::string bin_net_range(bin_net.begin(), bin_net.begin() + range);
      std::string addr_net_range(bin_addr.begin(), bin_addr.begin() + range);
      return (bin_net_range == addr_net_range);
    }
  }
  return false;
}

bool Setup::CheckLocalNet(const std::string& ip)
{
  bool isLocalNet = CheckIpAddress( ip, m_localnetmask, m_localnetmaskIPv6);
  if (!isLocalNet && m_allowlocalhost ) {
    // TNTnet unfortunately reports 'ip6-localhost' as "::" instead of "::1"
    isLocalNet = ip.compare("::") == 0 || ip.compare("::1") == 0 || CheckIpAddress( ip, "127.0.0.1/32", "0:0:0:0:0:0:0:1/128");
  }
  return m_islocalnet = isLocalNet;
}

bool Setup::SaveSetup()
{
  if (!liveplugin) return false;
  liveplugin->SetupStore("LastChannel", m_lastChannel);
  liveplugin->SetupStore("UseAuth", m_useAuth);
  if (m_useAuth)
  {
    liveplugin->SetupStore("AdminLogin", m_adminLogin.c_str());
    liveplugin->SetupStore("AdminPasswordMD5", m_adminPasswordMD5.c_str());
    liveplugin->SetupStore("LocalNetMask", m_localnetmask.c_str());
    liveplugin->SetupStore("LocalNetMaskIPv6", m_localnetmaskIPv6.c_str());
    liveplugin->SetupStore("AllowLocalhost", m_allowlocalhost);
  }
  liveplugin->SetupStore("UserdefTimes", m_times.c_str());
  liveplugin->SetupStore("ChannelGroupsGen", m_channelGroupsGen.c_str());
  liveplugin->SetupStore("ChannelGroups", m_channelGroups.c_str());
  liveplugin->SetupStore("ScheduleDuration", m_scheduleDuration.c_str());
  liveplugin->SetupStore("StartPage", m_startscreen.c_str());
  liveplugin->SetupStore("Theme", m_theme.c_str());
  liveplugin->SetupStore("LastWhatsOnListMode", m_lastwhatsonlistmode.c_str());
  liveplugin->SetupStore("LastSortingMode", m_lastsortingmode.c_str());
  liveplugin->SetupStore("ShowLogo", m_showLogo);
  liveplugin->SetupStore("ShowInfoBox", m_showInfoBox);
  liveplugin->SetupStore("UseStreamdev", m_useStreamdev);
  liveplugin->SetupStore("StreamdevPort", m_streamdevPort);
  liveplugin->SetupStore("StreamdevType", m_streamdevType.c_str());
  liveplugin->SetupStore("ScreenShotInterval", m_screenshotInterval);
  liveplugin->SetupStore("MarkNewRec", m_markNewRec);
  liveplugin->SetupStore("ShowIMDb", m_showIMDb);
  liveplugin->SetupStore("ShowChannelsWithoutEPG", m_showChannelsWithoutEPG);

  return true;
}

Setup& LiveSetup()
{
  static Setup instance;
  return instance;
}

cMenuSetupLive::cMenuSetupLive():
    cMenuSetupPage()
{
  m_lastChannel = vdrlive::LiveSetup().GetLastChannel();
  m_useAuth = vdrlive::LiveSetup().UseAuth();
  strcpy(m_adminLogin, vdrlive::LiveSetup().GetAdminLogin().c_str());

  m_oldpasswordMD5 = m_newpasswordMD5 = vdrlive::LiveSetup().GetMD5HashAdminPassword();

  std::string strHidden(vdrlive::LiveSetup().GetAdminPasswordLength(), '*');
  strn0cpy(m_tmpPassword, strHidden.c_str(), sizeof(m_tmpPassword));
  strcpy(m_adminPassword, "");
  Set();
}

void cMenuSetupLive::Set(void)
{
  int current = Current();
  Clear();
  //Add(new cMenuEditIntItem(tr("Last channel to display"),  &m_lastChannel, 0, 65536));
  Add(new cMenuEditChanItem(tr("Last channel to display"), &m_lastChannel, tr("No limit")));
  //Add(new cMenuEditIntItem(tr("Screenshot interval"),  &m_lastChannel, 0, 65536));
  Add(new cMenuEditBoolItem(tr("Use authentication"), &m_useAuth, tr("No"), tr("Yes")));
  Add(new cMenuEditStrItem( tr("Admin login"), m_adminLogin, sizeof(m_adminLogin), tr(FileNameChars)));
  Add(new cMenuEditStrItem( tr("Admin password"), m_tmpPassword, sizeof(m_tmpPassword), tr(FileNameChars)));
  SetCurrent(Get(current));
  Display();
}

void cMenuSetupLive::Store(void)
{
  vdrlive::LiveSetup().SetLastChannel(m_lastChannel);
  vdrlive::LiveSetup().SetUseAuth(m_useAuth);
  vdrlive::LiveSetup().SetAdminLogin(m_adminLogin);
  if (m_oldpasswordMD5 != m_newpasswordMD5) // only save the password if needed
    vdrlive::LiveSetup().SetAdminPassword(m_adminPassword);
  LiveSetup().SaveSetup();
}

bool cMenuSetupLive::InEditMode(const char* ItemText, const char* ItemName, const char* ItemValue)
{
  bool bEditMode = true;
  // ugly solution to detect, if in edit mode
  char* value = strdup(ItemText);
  strreplace(value, ItemName, "");
  strreplace(value, ":\t", "");
  // for bigpatch
  strreplace(value, "\t", "");
  if (strlen(value) == strlen(ItemValue))
    bEditMode = false;
  free(value);
  return bEditMode;
}

eOSState cMenuSetupLive::ProcessKey(eKeys Key)
{
  const char* ItemText = Get(Current())->Text();
  bool bPassWasInEditMode = false;
  if (ItemText && strlen(ItemText) > 0 && strstr(ItemText, tr("Admin password")) == ItemText)
    bPassWasInEditMode = InEditMode(ItemText, tr("Admin password"), m_tmpPassword);

  eOSState state = cMenuSetupPage::ProcessKey(Key);

  ItemText = Get(Current())->Text();
  bool bPassIsInEditMode = false;
  if (ItemText && strlen(ItemText) > 0 && strstr(ItemText, tr("Admin password")) == ItemText)
    bPassIsInEditMode = InEditMode(ItemText, tr("Admin password"), m_tmpPassword);

  if (bPassWasInEditMode && !bPassIsInEditMode)
  {
    strcpy(m_adminPassword, m_tmpPassword);
    m_newpasswordMD5 = MD5Hash(m_tmpPassword);
    std::string strHidden(strlen(m_adminPassword), '*');
    strcpy(m_tmpPassword, strHidden.c_str());
    Set();
    Display();
  }
  if (!bPassWasInEditMode && bPassIsInEditMode)
  {
    strcpy(m_tmpPassword, "");
    Set();
    Display();
    state = cMenuSetupPage::ProcessKey(Key);
  }

  return state;
}

} // namespace vdrlive


