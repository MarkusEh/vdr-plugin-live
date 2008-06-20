#ifndef VDR_LIVE_TNTCONFIG_H
#define VDR_LIVE_TNTCONFIG_H

#include <string>
#include <tnt/tntnet.h>

namespace vdrlive {

#if TNTVERSION >= 1606
	class TntConfig
	{
		public:
			static TntConfig const& Get();

			void Configure(tnt::Tntnet& app) const;

		private:
			TntConfig();
			TntConfig( TntConfig const& );
	};
#else
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
#endif

} // namespace vdrlive

#endif // VDR_LIVE_TNTCONFIG_H
