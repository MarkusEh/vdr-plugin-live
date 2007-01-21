#include "tools.h"

#include "epg_events.h"

namespace vdrlive
{
	EpgEvent::EpgEvent(const std::string& id,
					   const std::string& caption,
					   const std::string& title,
					   const std::string& short_descr,
					   const std::string& long_descr,
					   time_t start_time,
					   time_t end_time) :
		m_eventId(id),
		m_caption(caption),
		m_title(title),
		m_short_descr(short_descr),
		m_long_descr(long_descr),
		m_start_time(start_time),
		m_end_time(end_time)
	{
	}

	EpgEvent::EpgEvent(const std::string& id, const cEvent* event, const char* channelName) :
		m_eventId(id),
		m_caption(channelName),
		m_title(event->Title() ? event->Title() : ""),
		m_short_descr(event->ShortText() ? event->ShortText() : ""),
		m_long_descr(event->Description() ? event->Description() : ""),
		m_start_time(event->StartTime()),
		m_end_time(event->EndTime())
	{
	}

	EpgEvent::~EpgEvent()
	{
	}

	const std::string EpgEvent::StartTime(const char* format) const
	{
		return FormatDateTime(format, m_start_time);
	}

	const std::string EpgEvent::EndTime(const char* format) const
	{
		return FormatDateTime(format, m_end_time);
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
