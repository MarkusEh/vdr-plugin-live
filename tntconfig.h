#ifndef VDR_LIVE_TNTCONFIG_H
#define VDR_LIVE_TNTCONFIG_H

#include <string>

namespace vdrlive {

class TntConfig
{
public:
	static TntConfig const& Get();

	std::string const& GetConfigPath() const { return m_configPath; }

private:
	std::string m_propertiesPath;
	std::string m_configPath;

	TntConfig();
	TntConfig( TntConfig const& );

	void WriteProperties();
	void WriteConfig();
};

} // namespace vdrlive

#endif // VDR_LIVE_TNTCONFIG_H
