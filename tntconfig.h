#ifndef VDR_LIVE_TNTCONFIG_H
#define VDR_LIVE_TNTCONFIG_H

#include <string>
#include <tnt/tntnet.h>
#include <vdr/config.h>
#include "tntfeatures.h"

namespace vdrlive {

#if TNT_CONFIG_INTERNAL
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
#if APIVERSNUM > 10729
			std::string const& GetResourcePath() const { return m_resourcePath; }
#endif

		private:
			std::string m_propertiesPath;
			std::string m_configPath;
#if APIVERSNUM > 10729
			std::string m_resourcePath;
#endif

			TntConfig();
			TntConfig( TntConfig const& );

			void WriteProperties();
			void WriteConfig();
	};
#endif // TNT_CONFIG_INTERNAL

} // namespace vdrlive

#endif // VDR_LIVE_TNTCONFIG_H
