#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
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
		m_lastChannel( 0 ),
		m_screenshotInterval( 1000 ),
		m_useAuth( 1 ),
		m_adminLogin("admin")		
{
	m_adminPasswordMD5 = "4:" + MD5Hash("live");
}

bool Setup::ParseCommandLine( int argc, char* argv[] )
{
	static struct option opts[] = {
			{ "port", required_argument, NULL, 'p' },
			{ "ip",   required_argument, NULL, 'i' },
			{ 0 }
	};

	int optchar, optind = 0;
	while ( ( optchar = getopt_long( argc, argv, "p:i:", opts, &optind ) ) != -1 ) {
		switch ( optchar ) {
		case 'p': m_serverPort = atoi( optarg ); break;
		case 'i': m_serverIps.push_back( optarg ); break;
		default:  return false;
		}
	}

	return CheckServerPort() &&
		   CheckServerIps();
}

char const* Setup::CommandLineHelp() const
{
	if ( m_helpString.empty() ) {
		ostringstream builder;
		builder << "  -p PORT,  --port=PORT     use PORT to listen for incoming connections\n"
				   "                            (default: " << m_serverPort << ")\n"
				<< "  -i IP,    --ip=IP         bind server only to specified IP, may appear\n"
				   "                            multiple times\n"
				   "                            (default: 0.0.0.0)\n";
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
	else return false;
	return true;
}

bool Setup::CheckServerPort()
{
	if ( m_serverPort <= 0 || m_serverPort > numeric_limits< uint16_t >::max() ) {
		esyslog( "ERROR: live server port %d is not a valid port number", m_serverPort );
		cerr << "ERROR: live server port " << m_serverPort << " is not a valid port number" << endl;
		return false;
	}
	return true;
}

bool Setup::CheckServerIps()
{
	if ( m_serverIps.empty() ) {
		m_serverIps.push_back( "0.0.0.0" );
		return true;
	}

	for ( IpList::const_iterator ip = m_serverIps.begin(); ip != m_serverIps.end(); ++ip ) {
		if ( inet_addr( ip->c_str() ) == static_cast< in_addr_t >( -1 ) ) {
			esyslog( "ERROR: live server ip %s is not a valid ip address", ip->c_str() );
			cerr << "ERROR: live server ip " << *ip << " is not a valid ip address" << endl;
			return false;
		}
	}
	return true;
}

bool Setup::HaveEPGSearch(void)
{
	return cPluginManager::GetPlugin("epgsearch") != NULL;	
}

std::string Setup::GetMD5HashAdminPassword() const
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
	SetupStore("LastChannel",  m_lastChannel);
	
	vdrlive::LiveSetup().SetUseAuth(m_useAuth);
	SetupStore("UseAuth",  m_useAuth);
	
	vdrlive::LiveSetup().SetAdminLogin(m_adminLogin);
	SetupStore("AdminLogin",  m_adminLogin);
	
	if (m_oldpasswordMD5 != m_newpasswordMD5) // only save the password if needed
	{
		std::string passwordMD5 = vdrlive::LiveSetup().SetAdminPassword(m_adminPassword);
		SetupStore("AdminPasswordMD5",  passwordMD5.c_str());
	}
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


