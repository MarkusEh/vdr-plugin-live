#ifndef VDR_LIVE_TNTCONFIG_H
#define VDR_LIVE_TNTCONFIG_H

#if TNTVERSION >= 30000
  #include <fstream>
#endif

#include "tntfeatures.h"

#include <tnt/tntnet.h>

namespace vdrlive {

  class TntConfig
  {
    public:
      static TntConfig const& Get();

                        void ConfigureTvscraper(tnt::Tntnet& app, const std::string &tvscraperImageDir) const;
        void Configure(tnt::Tntnet& app) const;

    private:
      TntConfig();
      TntConfig( TntConfig const& );
  };

} // namespace vdrlive

#endif // VDR_LIVE_TNTCONFIG_H
