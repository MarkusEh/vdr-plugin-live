#include "tools.h"

#include "epg_events.h"

namespace vdrlive
{
	EpgEvent::EpgEvent(const std::string& id, const cEvent* event, const char* channelName) :
		eventId(id),
		title(event->Title() ? event->Title() : ""),
		channel_name(channelName),
		short_descr(event->ShortText() ? event->ShortText() : ""),
		long_descr(event->Description() ? event->Description() : ""),
		start_time(event->StartTime()),
		end_time(event->EndTime())
	{
	}

	EpgEvent::~EpgEvent()
	{
	}

	const std::string EpgEvent::StartTime(const char* format) const
	{
		return FormatDateTime(format, start_time);
	}

	const std::string EpgEvent::EndTime(const char* format) const
	{
		return FormatDateTime(format, end_time);
	}

	EpgEvents::EpgEvents() :
		std::vector<EpgEventPtr>()
	{
	}

	EpgEvents::~EpgEvents()
	{
	}
#ifdef never
	EpgEventsPtr EpgEvents::dim(size_t count)
	{
		EpgEventsPtr ePtr(new EpgEvents(count));
		return ePtr;
	}
#endif
}; // namespace vdrlive
