#ifndef VDR_LIVE_TNTCONFIG_H
#define VDR_LIVE_TNTCONFIG_H

#include <string>
#include <tnt/tntnet.h>
#include <vdr/config.h>
#include "tntfeatures.h"

namespace vdrlive {

	class TntConfig
	{
		public:
			static TntConfig const& Get();

			void Configure(tnt::Tntnet& app) const;

		private:
			TntConfig();
			TntConfig( TntConfig const& );
	};

} // namespace vdrlive

#endif // VDR_LIVE_TNTCONFIG_H
