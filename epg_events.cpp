
#include "epg_events.h"

#include "tools.h"
#include "recman.h"
#include "setup.h"

// STL headers need to be before VDR tools.h (included by <vdr/player.h>)
#include <glob.h>
#include <cassert>

#include <vdr/player.h>

#ifndef TVM2VDR_PL_WORKAROUND
#define TVM2VDR_PL_WORKAROUND  0
#endif

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

	int EpgInfo::Duration() const
	{
		return EpgEvents::Duration(GetStartTime(), GetEndTime());
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
		if (!m_checkedArchived && m_recording) {
			m_archived = RecordingsManager::GetArchiveDescr(m_recording);
			m_checkedArchived = true;
		}
		return m_archived;
	}

	time_t EpgRecording::GetStartTime() const
	{
		return m_recording ? m_recording->Start() : 0;
	}

	time_t EpgRecording::GetEndTime() const
	{
		time_t endTime = 0;
		if (m_recording)
		{
			time_t startTime = m_recording->Start();
			int length = m_recording->LengthInSeconds();

			endTime = (length < 0) ? startTime : startTime + length;
		}
		return endTime;
	}

	int EpgRecording::Elapsed() const
	{
		cControl* pControl = cControl::Control();
		if (pControl)
		{
			int current, total;
			if (pControl->GetIndex(current,total))
			{
				if (total)
				{
					return (100 * current) / total;
				}
			}
		}
		return 0;
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
	 * EmptyEvent
	 * -------------------------------------------------------------------------
	 */

	EmptyEvent::EmptyEvent(std::string const &id, tChannelID const &channelID, const char* channelName) :
		EpgInfo(id, channelName),
		m_channelID(channelID)
	{
	}

	EmptyEvent::~EmptyEvent()
	{
	}

	/*
	 * -------------------------------------------------------------------------
	 * EpgEvents
	 * -------------------------------------------------------------------------
	 */
	namespace EpgEvents {
		string EncodeDomId(tChannelID const &chanId, tEventID const &eId)
		{
			string channelId(chanId.ToString());
			string eventId("event_");

			channelId = vdrlive::EncodeDomId(channelId, ".-", "pm");

			eventId += channelId;
			eventId += '_';
			eventId += lexical_cast<std::string>(eId);
			return eventId;
		}

		void DecodeDomId(string const &epgid, tChannelID& channelId, tEventID& eventId)
		{
			string const eventStr("event_");

			size_t delimPos = epgid.find_last_of('_');
			string cIdStr = epgid.substr(eventStr.length(), delimPos - eventStr.length());

			cIdStr = vdrlive::DecodeDomId(cIdStr, "mp", "-.");

			string const eIdStr = epgid.substr(delimPos+1);

			channelId = tChannelID::FromString(cIdStr.c_str());
			eventId = lexical_cast<tEventID>(eIdStr);
		}

		EpgInfoPtr CreateEpgInfo(string const &epgid, cSchedules const *schedules)
		{
			string const errorInfo(tr("Epg error"));

			tEventID eventId = tEventID();
			tChannelID channelId = tChannelID();

			DecodeDomId(epgid, channelId, eventId);
#if VDRVERSNUM >= 20301
			LOCK_CHANNELS_READ;
			cChannel const *channel = Channels->GetByChannelID(channelId);
#else
			cChannel const *channel = Channels.GetByChannelID(channelId);
#endif
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

		EpgInfoPtr CreateEpgInfo(cChannel const *chan, cEvent const *event, char const *idOverride)
		{
			assert(chan);

			if (event) {
				string domId(idOverride ? idOverride : EncodeDomId(chan->GetChannelID(), event->EventID()));
				return EpgInfoPtr(new EpgEvent(domId, event, chan->Name()));
			}
			if (LiveSetup().GetShowChannelsWithoutEPG()) {
				string domId(idOverride ? idOverride : EncodeDomId(chan->GetChannelID(), 0));
				return EpgInfoPtr(new EmptyEvent(domId, chan->GetChannelID(), chan->Name()));
			}
			return EpgInfoPtr();
		}

		EpgInfoPtr CreateEpgInfo(string const &recid, cRecording const *recording, char const *caption)
		{
			return EpgInfoPtr(new EpgRecording(recid, recording, caption));
		}

		EpgInfoPtr CreateEpgInfo(string const &id, string const &caption, string const &info)
		{
			return EpgInfoPtr(new EpgString(id, caption, info));
		}


		bool ScanForEpgImages(string const & imageId, string const & wildcard, list<string> & images)
		{
			bool found = false;
			const string filemask(LiveSetup().GetEpgImageDir() + "/" + imageId + wildcard);
			glob_t globbuf;
			globbuf.gl_offs = 0;
			if (!LiveSetup().GetEpgImageDir().empty() && glob(filemask.c_str(), GLOB_DOOFFS, NULL, &globbuf) == 0) {
				for(size_t i = 0; i < globbuf.gl_pathc; i++) {
					const string imagefile(globbuf.gl_pathv[i]);
					size_t delimPos = imagefile.find_last_of('/');
					images.push_back(imagefile.substr(delimPos+1));
					found = true;
				}
				globfree(&globbuf);
			}
			return found;
		}

		list<string> EpgImages(string const &epgid)
		{
			size_t delimPos = epgid.find_last_of('_');
			string imageId = epgid.substr(delimPos+1);

			list<string> images;

			// Initially we scan for images that follow the scheme
			// '<epgid>_<distinction>.*' where distincition is any
			// character sequence.  Usually distinction will be used
			// to assign more than one image to an epg event. Thus it
			// will be a digit or number.  The sorting of the images
			// will depend on the 'distinction' lexical sorting
			// (similar to what ls does).
			// Example:
			//   112123_0.jpg		first epg image for event id 112123
			//   112123_1.png		second epg image for event id 112123
			if (! ScanForEpgImages(imageId, "_*.*", images))
			{
				// if we didn't find images that follow the scheme
				// above we try to find images that contain only the
				// event id as file name without extension:
				if (! ScanForEpgImages(imageId, ".*", images))
				{
#if TVM2VDR_PL_WORKAROUND
					// if we didn't get images try to work arround a
					// bug in tvm2vdr.  tvm2vdr seems always to use
					// one digit less, which leads in some rare cases
					// to the bug in LIVE, that unrelated and to many
					// images are displayed.  But without this 'fix'
					// no images would be visible at all. The bug
					// should be fixed in tvm2vdr.pl (Perl version of
					// tvm2vdr).  There exists a plugin - also called
					// tvm2vdr - which does not have that bug.
					imageId = imageId.substr(0, imageId.size()-1);
					ScanForEpgImages(imageId, "*.*", images);
#endif
				}
			}
			return images;
		}

		int ElapsedTime(time_t const startTime, time_t const endTime)
		{
			// Elapsed time is only meaningful when there is a non zero
			// duration (e.g. startTime != endTime and endTime > startTime)
			int duration = Duration(startTime, endTime);
			if (duration > 0) {
				time_t now = time(0);
				if ((startTime <= now) && (now <= endTime)) {
					return 100 * (now - startTime) / duration;
				}
			}
			return -1;
		}

		int Duration(time_t const startTime, time_t const endTime)
		{
			return endTime - startTime;
		}

	} // namespace EpgEvents

}; // namespace vdrlive
