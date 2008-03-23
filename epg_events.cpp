#include <time.h>
#include <glob.h>
#include <algorithm>

#include "tools.h"
#include "recman.h"

#include "epg_events.h"
#include "setup.h"

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

	const string EpgInfo::CurrentTime(const char* format) const
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
		return EpgEvents::ElapsedTime(GetStartTime(), GetEndTime());
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
		if (m_ownCaption) {
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
			name = name.substr(index+1);
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

	string EpgEvents::EncodeDomId(tChannelID const &chanId, tEventID const &eId)
	{
		string channelId(chanId.ToString());
		string eventId("event_");

		channelId = vdrlive::EncodeDomId(channelId, ".-", "pm");
		// replace(channelId.begin(), channelId.end(), '.', 'p');
		// replace(channelId.begin(), channelId.end(), '-', 'm');

		eventId += channelId;
		eventId += '_';
		eventId += lexical_cast<std::string>(eId);
		return eventId;
	}

	void EpgEvents::DecodeDomId(string const &epgid, tChannelID& channelId, tEventID& eventId)
	{
		string const eventStr("event_");

		size_t delimPos = epgid.find_last_of('_');
		string cIdStr = epgid.substr(eventStr.length(), delimPos - eventStr.length());

		cIdStr = vdrlive::DecodeDomId(cIdStr, "mp", "-.");
		// replace(cIdStr.begin(), cIdStr.end(), 'm', '-');
		// replace(cIdStr.begin(), cIdStr.end(), 'p', '.');

		string const eIdStr = epgid.substr(delimPos+1);

		channelId = tChannelID::FromString(cIdStr.c_str());
		eventId = lexical_cast<tEventID>(eIdStr);
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(string const &epgid, cSchedules const *schedules)
	{
		string const errorInfo(tr("Epg error"));
		cSchedulesLock schedulesLock;

		tEventID eventId = tEventID();
		tChannelID channelId = tChannelID();

		DecodeDomId(epgid, channelId, eventId);
		cChannel const *channel = Channels.GetByChannelID(channelId);
		if (!channel) {
			return CreateEpgInfo(epgid, errorInfo, tr("Wrong channel id"));
		}
		cSchedule const *schedule = schedules->GetSchedule(channel);
		if (!schedule) {
			return CreateEpgInfo(epgid, errorInfo, tr("Channel has no schedule"));
		}
		cEvent const *event = schedule->GetEvent(eventId);
		if (!event) {
			return CreateEpgInfo(epgid, errorInfo, tr("Wrong event id"));
		}
		return CreateEpgInfo(channel, event, epgid.c_str());
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(cChannel const *chan, cEvent const *event, char const *idOverride)
	{
		string domId(idOverride ? idOverride : EncodeDomId(chan->GetChannelID(), event->EventID()));
		return EpgInfoPtr(new EpgEvent(domId, event, chan->Name()));
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(string const &recid, cRecording const *recording, char const *caption)
	{
		return EpgInfoPtr(new EpgRecording(recid, recording, caption));
	}

	EpgInfoPtr EpgEvents::CreateEpgInfo(string const &id, string const &caption, string const &info)
	{
		return EpgInfoPtr(new EpgString(id, caption, info));
	}

	list<string> EpgEvents::EpgImages(string const &epgid)
	{
		list<string> images;

		size_t delimPos = epgid.find_last_of('_');
		string imageId = epgid.substr(delimPos+1);
		imageId = imageId.substr(0, imageId.size()-1); // tvm2vdr seems always to use one digit less

		const string filemask(LiveSetup().GetEpgImageDir() + "/" + imageId + "*.*");
		glob_t globbuf;
		globbuf.gl_offs = 0;
		if (!LiveSetup().GetEpgImageDir().empty() && glob(filemask.c_str(), GLOB_DOOFFS, NULL, &globbuf) == 0)
		{
			for(int i=0; i<(int)globbuf.gl_pathc; i++)
			{
				const string imagefile(globbuf.gl_pathv[i]);
				size_t delimPos = imagefile.find_last_of('/');
				images.push_back(imagefile.substr(delimPos+1));
			}
			globfree(&globbuf);
		}
		return images;
	}

	int EpgEvents::ElapsedTime(time_t const startTime, time_t const endTime)
	{
		if (endTime > startTime) {
			time_t now = time(0);
			if ((startTime <= now) && (now <= endTime)) {
				return 100 * (now - startTime) / (endTime - startTime);
			}
		}
		return -1;
	}
}; // namespace vdrlive
