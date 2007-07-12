#include <time.h>

#include "tools.h"
#include "recordings.h"

#include "epg_events.h"

using namespace std;

namespace vdrlive
{

	/*
	 * -------------------------------------------------------------------------
	 * EpgInfo
	 * -------------------------------------------------------------------------
	 */

	EpgInfo::EpgInfo(const std::string& id, const std::string& caption) :
		m_eventId(id),
		m_caption(caption)
	{
	}

	EpgInfo::~EpgInfo()
	{
	}

	const std::string EpgInfo::CurrentTime(const char* format) const
	{
		return FormatDateTime(format, time(0));
	}

	const string EpgInfo::StartTime(const char* format) const
	{
		time_t start = GetStartTime();
		return start ? FormatDateTime(format, start) : "";
	}

	const string EpgInfo::EndTime(const char* format) const
	{
		time_t end = GetEndTime();
		return end ? FormatDateTime(format, end) : "";
	}

	int EpgInfo::Elapsed() const
	{
		time_t end_time = GetEndTime();
		time_t start_time = GetStartTime();

		if (end_time > start_time) {
			time_t now = time(0);
			if ((start_time <= now) && (now <= end_time)) {
				return 100 * (now - start_time) / (end_time - start_time);
			}
		}
		return -1;
	}

	/*
	 * -------------------------------------------------------------------------
	 * EpgEvent
	 * -------------------------------------------------------------------------
	 */

	EpgEvent::EpgEvent(const std::string& id, const cEvent* event, const char* channelName) :
		EpgInfo(id, channelName),
		m_event(event)
	{
	}

	EpgEvent::~EpgEvent()
	{
	}

	/*
	 * -------------------------------------------------------------------------
	 * EpgString
	 * -------------------------------------------------------------------------
	 */

	EpgString::EpgString(const string& id, const string& caption, const string& info) :
		EpgInfo(id, caption),
		m_info(info)
	{
	}

	EpgString::~EpgString()
	{
	}

	const string EpgString::Title() const
	{
		return m_info;
	}

	const string EpgString::ShortDescr() const
	{
		return "";
	}

	const string EpgString::LongDescr() const
	{
		return "";
	}

	// virtual const std::string Archived() const { return std::string(); }

	time_t EpgString::GetStartTime() const
	{
		return time(0);
	}

	time_t EpgString::GetEndTime() const
	{
		return time(0);
	}

	/*
	 * -------------------------------------------------------------------------
	 * EpgRecording
	 * -------------------------------------------------------------------------
	 */

	EpgRecording::EpgRecording(const string& recid, const cRecording* recording, const char* caption) :
		EpgInfo(recid, (caption != 0) ? caption : ""),
		m_recording(recording),
		m_ownCaption(caption != 0),
		m_checkedArchived(false),
		m_archived()
	{
	}

	EpgRecording::~EpgRecording()
	{
		m_recording = 0;
	}

	const string EpgRecording::Caption() const
	{
		if (!m_ownCaption) {
			return EpgInfo::Caption();
		}
		if (!m_recording) {
			return "";
		}

		return Name();
	}

	const string EpgRecording::Title() const
	{
		if (!m_recording) {
			return "";
		}

		const cRecordingInfo* info = m_recording->Info();
		return (info && info->Title()) ? info->Title() : Name();
	}

	const string EpgRecording::ShortDescr() const
	{
		const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
		return (info && info->ShortText()) ? info->ShortText() : "";
	}

	const string EpgRecording::LongDescr() const
	{
		const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
		return (info && info->Description()) ? info->Description() : "";
	}

	const string EpgRecording::Archived() const
	{
		if (!m_checkedArchived) {
			if (m_recording) {
				m_archived = RecordingsManager::GetArchiveDescr(m_recording);
				m_checkedArchived = true;
			}
		}
		return m_archived;
	}

	time_t EpgRecording::GetStartTime() const
	{
		return m_recording ? m_recording->start : 0;
	}

	time_t EpgRecording::GetEndTime() const
	{
		return m_recording ? m_recording->start : 0;
	}

	const string EpgRecording::Name() const
	{
		string name(m_recording->Name());
		size_t index = name.find_last_of('~');
		if (index != string::npos) {
			name = name.substr(index, name.length());
		}
		return name;
	}

	/*
	 * -------------------------------------------------------------------------
	 * EpgEvents
	 * -------------------------------------------------------------------------
	 */

	EpgEvents::EpgEvents()
	{
	}

	EpgEvents::~EpgEvents()
	{
	}

	string EpgEvents::GetDomId(const tChannelID& chanId, const tEventID& eId)
	{
		string channelId(chanId.ToString());
		string eventId("event_");

		replace(channelId.begin(), channelId.end(), '.', 'p');
		replace(channelId.begin(), channelId.end(), '-', 'm');

		eventId += channelId;
		eventId += '_';
		eventId += lexical_cast<std::string>(eId);
		return eventId;
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(const std::string& epgid, const cSchedules* schedules)
	{
		const string eventStr("event_");

		size_t delimPos = epgid.find_last_of('_');
		string cIdStr = epgid.substr(eventStr.length(), delimPos - eventStr.length());

		replace(cIdStr.begin(), cIdStr.end(), 'm', '-');
		replace(cIdStr.begin(), cIdStr.end(), 'p', '.');

		const string eIdStr = epgid.substr(delimPos+1);
		const string errorInfo(tr("Epg error"));


		tEventID eventId = lexical_cast<tEventID>(eIdStr);
		tChannelID channelId = tChannelID::FromString(cIdStr.c_str());
		const cChannel* channel = Channels.GetByChannelID(channelId);
		if (!channel) {
			return CreateEpgInfo(epgid, errorInfo, tr("Wrong channel id"));
		}
		const cSchedule* schedule = schedules->GetSchedule(channel);
		if (!schedule) {
			return CreateEpgInfo(epgid, errorInfo, tr("Channel has no schedule"));
		}
		const cEvent* event = schedule->GetEvent(eventId);
		if (!event) {
			return CreateEpgInfo(epgid, errorInfo, tr("wrong event id"));
		}
		return CreateEpgInfo(channel, event, epgid.c_str());
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(const cChannel* chan, const cEvent* event, const char* idOverride)
	{
		string domId(idOverride ? idOverride : GetDomId(chan->GetChannelID(), event->EventID()));
		return EpgInfoPtr(new EpgEvent(domId, event, chan->Name()));
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(const string& recid, const cRecording* recording, const char* caption)
	{
		return EpgInfoPtr(new EpgRecording(recid, recording, caption));
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(const std::string& id, const std::string& caption, const std::string& info)
	{
		return EpgInfoPtr(new EpgString(id, caption, info));
	}
}; // namespace vdrlive
