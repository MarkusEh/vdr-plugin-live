#ifndef VDR_LIVE_WHATS_ON_H
#define VDR_LIVE_WHATS_ON_H

#include <ctime>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <vdr/plugin.h>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/config.h>
#include <vdr/i18n.h>

#include "live.h"

namespace vdrlive
{

	class EpgEvent
	{
		public:
			EpgEvent(const std::string& id, const cEvent* event, const char* channelName = "");

			virtual ~EpgEvent();

			const std::string& Id() const { return eventId; }

			const std::string& Title() const { return title; }

			const std::string& ChannelName() const { return channel_name; }

			const std::string& ShortDescr() const { return short_descr; }

			const std::string& LongDescr() const { return long_descr; }

			const std::string StartTime(const char* format) const;

			const std::string EndTime(const char* format) const;

		private:
			std::string eventId;
			std::string title;
			std::string channel_name;
			std::string short_descr;
			std::string long_descr;
			time_t start_time;
			time_t end_time;
	};

	typedef boost::shared_ptr<EpgEvent> EpgEventPtr;

	class EpgEvents : public std::vector<EpgEventPtr> {
		public:
			EpgEvents();
			virtual ~EpgEvents();

		private:
	};
}; // namespace vdrlive

#endif // VDR_LIVE_WHATS_ON_H

