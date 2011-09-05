#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <bitset>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <arpa/inet.h>
#include <vdr/tools.h>
#include <vdr/menuitems.h>
#include <vdr/plugin.h>
#include "setup.h"
#include "tools.h"

namespace vdrlive {

using namespace std;

Setup::Setup():
		m_serverPort( 8008 ),
#if TNT_SSL_SUPPORT
		m_serverSslPort( 8443 ),
		m_serverSslCert(),
		m_serverSslKey(),
#endif
		m_lastChannel( 0 ),
		m_screenshotInterval( 1000 ),
		m_useAuth( 1 ),
		m_adminLogin("admin"),
		m_channelGroups( "" ),
		m_scheduleDuration( "8" ),
		m_theme("marine"),
		m_lastwhatsonlistmode("detail"),
		m_lastsortingmode("nameasc"),
		m_tntnetloglevel("WARN"),
		m_showLogo(1),
		m_useAjax(1),
		m_showInfoBox(1),
		m_useStreamdev(1),
		m_streamdevPort(3000),
		m_streamdevType(),
		m_showIMDb(1),
		m_showChannelsWithoutEPG(0)
{
	m_adminPasswordMD5 = "4:" + MD5Hash("live");
	liveplugin = cPluginManager::GetPlugin("live");
}

bool Setup::ParseCommandLine( int argc, char* argv[] )
{
	static struct option opts[] = {
			{ "port", required_argument, NULL, 'p' },
			{ "ip",   required_argument, NULL, 'i' },
			{ "log",  required_argument, NULL, 'l' },
			{ "epgimages",  required_argument, NULL, 'e' },
#if TNT_SSL_SUPPORT
			{ "sslport", required_argument, NULL, 's' },
			{ "cert", required_argument, NULL, 'c' },
			{ "key", required_argument, NULL, 'k' },
#endif
			{ 0 }
	};

	int optchar, optind = 0;
	while ( ( optchar = getopt_long( argc, argv, "p:i:l:e:s:c:", opts, &optind ) ) != -1 ) {
		switch ( optchar ) {
		case 'p': m_serverPort = atoi( optarg ); break;
		case 'i': m_serverIps.push_back( optarg ); break;
		case 'l': m_tntnetloglevel = optarg; break;
		case 'e': m_epgimagedir = optarg; break;
#if TNT_SSL_SUPPORT
		case 's': m_serverSslPort = atoi( optarg ); break;
		case 'c': m_serverSslCert = optarg; break;
		case 'k': m_serverSslKey = optarg; break;
#endif
		default:  return false;
		}
	}

	return CheckServerPort() &&
#if TNT_SSL_SUPPORT
		   CheckServerSslPort() &&
#endif
		   CheckServerIps();
}

char const* Setup::CommandLineHelp() const
{
	if ( m_helpString.empty() ) {
		ostringstream builder;
		builder         << "  -p PORT,  --port=PORT        use PORT to listen for incoming connections\n"
				   "                               (default: " << m_serverPort << ")\n"
				<< "  -i IP,    --ip=IP            bind server only to specified IP, may appear\n"
				   "                               multiple times\n"
				   "                               (default: 0.0.0.0)\n"
#if TNT_SSL_SUPPORT
				<< "  -s PORT,  --sslport=PORT     use PORT to listen for incoming ssl connections\n"
				   "                               (default: " << m_serverSslPort << ")\n"
				<< "  -c CERT,  --cert=CERT        full path to a custom ssl certificate file\n"
				<< "  -k KEY,  --key=KEY           full path to a custom ssl certificate key file\n"
#endif
				<< "  -l level, --log=level        log level for tntnet (values: WARN, ERROR, INFO, DEBUG, TRACE)\n"
				<< "  -e <dir>, --epgimages=<dir>  directory for epgimages\n";
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
	else if ( strcmp( name, "ChannelGroups" ) == 0 ) m_channelGroups = value;
	else if ( strcmp( name, "ScheduleDuration" ) == 0 ) m_scheduleDuration = value;
	else if ( strcmp( name, "StartPage" ) == 0 ) m_startscreen = value;
	else if ( strcmp( name, "Theme" ) == 0 ) m_theme = value;
	else if ( strcmp( name, "LocalNetMask" ) == 0 ) { m_localnetmask = value; }
	else if ( strcmp( name, "LastWhatsOnListMode" ) == 0 ) { m_lastwhatsonlistmode = value; }
	else if ( strcmp( name, "LastSortingMode" ) == 0 ) { m_lastsortingmode = value; }
	else if ( strcmp( name, "ShowLogo" ) == 0 ) { m_showLogo = atoi(value); }
	else if ( strcmp( name, "UseAjax" ) == 0 ) { m_useAjax = atoi(value); }
	else if ( strcmp( name, "ShowInfoBox" ) == 0 ) { m_showInfoBox = atoi(value); }
	else if ( strcmp( name, "UseStreamdev" ) == 0 ) { m_useStreamdev = atoi(value); }
	else if ( strcmp( name, "StreamdevPort" ) == 0 ) { m_streamdevPort = atoi(value); }
	else if ( strcmp( name, "StreamdevType" ) == 0 ) { m_streamdevType = value; }
	else if ( strcmp( name, "ScreenShotInterval" ) == 0 ) { m_screenshotInterval = atoi(value); }
	else if ( strcmp( name, "ShowIMDb" ) == 0 ) { m_showIMDb = atoi(value); }
	else if ( strcmp( name, "ShowChannelsWithoutEPG" ) == 0 ) { m_showChannelsWithoutEPG = atoi(value); }
	else return false;
	return true;
}

bool Setup::CheckServerPort()
{
	if ( m_serverPort <= 0 || m_serverPort > numeric_limits< uint16_t >::max() ) {
		esyslog( "[live] ERROR: server port %d is not a valid port number", m_serverPort );
		cerr << "ERROR: live server port " << m_serverPort << " is not a valid port number" << endl;
		return false;
	}
	return true;
}

#if TNT_SSL_SUPPORT
bool Setup::CheckServerSslPort()
{
	if ( m_serverSslPort <= 0 || m_serverSslPort > numeric_limits< uint16_t >::max() ) {
		esyslog( "[live] ERROR: server ssl port %d is not a valid port number", m_serverSslPort );
		cerr << "ERROR: live server ssl port " << m_serverSslPort << " is not a valid port number" << endl;
		return false;
	}
	return true;
}
#endif

namespace {
	struct IpValidator
	{
		bool operator() (string const & ip)
		{
			struct in6_addr buf;
			struct in_addr buf4;

			esyslog( "[live] INFO: validating server ip '%s'", ip.c_str());
			cerr << "INFO: validating live server ip '" << ip << "'" << endl;
			bool valid = inet_aton(ip.c_str(), &buf4) || inet_pton(AF_INET6, ip.c_str(), &buf);

			if (!valid) {
				esyslog( "[live] ERROR: server ip %s is not a valid ip address", ip.c_str());
				cerr << "ERROR: live server ip '" << ip << "' is not a valid ip address" << endl;
			}
			return valid;
		}
	};
}

bool Setup::CheckServerIps()
{
	if ( m_serverIps.empty() ) {
		FILE* f = fopen("/proc/sys/net/ipv6/bindv6only", "r");
		if (f) {
			bool bindv6only = false;
			int c = fgetc(f);
			if (c != EOF) {
				bindv6only = c - '0';
			}
			fclose(f);
			f = NULL;
			esyslog( "[live] INFO: bindv6only=%d", bindv6only);
			// add a default IPv6 listener address
			m_serverIps.push_back( "::" );
			// skip the default IPv4 listener address if IPv6 one will be binded also to v4
			if (!bindv6only)
				return true;
		}
		// add a default IPv4 listener address
		m_serverIps.push_back( "0.0.0.0" );
		// we assume these are ok :)
		return true;
	}

	IpList::iterator i = partition(m_serverIps.begin(), m_serverIps.end(), IpValidator());
	m_serverIps.erase(i, m_serverIps.end());

	return !m_serverIps.empty();
}

std::string const Setup::GetMD5HashAdminPassword() const
{
	// format is <length>:<md5-hash of password>
	vector< string > parts = StringSplit( m_adminPasswordMD5, ':' );
	return (parts.size() > 1) ? parts[1] : "";
}

int Setup::GetAdminPasswordLength() const
{
	// format is <length>:<md5-hash of password>
	vector< string > parts = StringSplit( m_adminPasswordMD5, ':' );
	return (parts.size() > 0) ? lexical_cast< int >( parts[0] ) : 0;
}

std::string Setup::SetAdminPassword(std::string password)
{
	ostringstream passwordStr;
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

bool Setup::CheckLocalNet(const std::string& ip)
{
	// split local net mask in net and range
	vector< string > parts = StringSplit( m_localnetmask, '/' );
	if (parts.size() != 2) return false;
	string net = parts[0];

	int range = lexical_cast< int >(parts[1]);
	// split net and ip addr in its 4 subcomponents
	vector< string > netparts = StringSplit( net, '.' );
	vector< string > addrparts = StringSplit( ip, '.' );
	if (netparts.size() != 4 || addrparts.size() != 4) return false;

	// to binary representation
	ostringstream bin_netstream;
	bin_netstream << bitset<8>(lexical_cast<long>(netparts[0]))
		<< bitset<8>(lexical_cast<long>(netparts[1]))
		<< bitset<8>(lexical_cast<long>(netparts[2]))
		<< bitset<8>(lexical_cast<long>(netparts[3]));

	ostringstream bin_addrstream;
	bin_addrstream << bitset<8>(lexical_cast<long>(addrparts[0]))
		<< bitset<8>(lexical_cast<long>(addrparts[1]))
		<< bitset<8>(lexical_cast<long>(addrparts[2]))
		<< bitset<8>(lexical_cast<long>(addrparts[3]));

	// compare range
	string bin_net = bin_netstream.str();
	string bin_addr = bin_addrstream.str();
	string bin_net_range(bin_net.begin(), bin_net.begin() + range);
	string addr_net_range(bin_addr.begin(), bin_addr.begin() + range);
	m_islocalnet = (bin_net_range == addr_net_range);

	return m_islocalnet;
}

bool Setup::SaveSetup()
{
	if (!liveplugin) return false;
	liveplugin->SetupStore("LastChannel",  m_lastChannel);
	liveplugin->SetupStore("UseAuth",  m_useAuth);
	if (m_useAuth)
	{
		liveplugin->SetupStore("AdminLogin",  m_adminLogin.c_str());
		liveplugin->SetupStore("AdminPasswordMD5",  m_adminPasswordMD5.c_str());
		liveplugin->SetupStore("LocalNetMask",  m_localnetmask.c_str());
	}
	liveplugin->SetupStore("UserdefTimes",  m_times.c_str());
	liveplugin->SetupStore("ChannelGroups",  m_channelGroups.c_str());
	liveplugin->SetupStore("ScheduleDuration",  m_scheduleDuration.c_str());
	liveplugin->SetupStore("StartPage",  m_startscreen.c_str());
	liveplugin->SetupStore("Theme", m_theme.c_str());
	liveplugin->SetupStore("LastWhatsOnListMode", m_lastwhatsonlistmode.c_str());
	liveplugin->SetupStore("LastSortingMode", m_lastsortingmode.c_str());
	liveplugin->SetupStore("ShowLogo", m_showLogo);
	liveplugin->SetupStore("UseAjax", m_useAjax);
	liveplugin->SetupStore("ShowInfoBox", m_showInfoBox);
	liveplugin->SetupStore("UseStreamdev", m_useStreamdev);
	liveplugin->SetupStore("StreamdevPort", m_streamdevPort);
	liveplugin->SetupStore("StreamdevType", m_streamdevType.c_str());
	liveplugin->SetupStore("ScreenShotInterval", m_screenshotInterval);
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

	string strHidden(vdrlive::LiveSetup().GetAdminPasswordLength(), '*');
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
		string strHidden(strlen(m_adminPassword), '*');
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


