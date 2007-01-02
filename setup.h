#ifndef VDR_LIVE_SETUP_H
#define VDR_LIVE_SETUP_H

#include <string>
#include <list>

namespace vdrlive {

class Setup
{
public:
	typedef std::list< std::string > IpList;

	static Setup& Get();

	std::string const& GetLibraryPath() const { return m_libraryPath; }
	int GetServerPort() const { return m_serverPort; }
	IpList const& GetServerIps() const { return m_serverIps; }

	bool Parse( int argc, char* argv[] );
	char const* Help() const;
	
private:
	Setup();
	Setup( Setup const& );

	mutable std::string m_helpString;
	std::string m_libraryPath;
	int m_serverPort;
	std::list< std::string > m_serverIps;

	bool CheckLibraryPath();
	bool CheckServerPort();
	bool CheckServerIps();
};

} // namespace vdrlive

#endif // VDR_LIVE_SETUP_H
