#ifndef VDR_LIVE_EPG_EVENTS_H
#define VDR_LIVE_EPG_EVENTS_H

#include "stdext.h"

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <string>
#include <list>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any vdr include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include "services.h"
#include "recman.h"
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/recording.h>

namespace vdrlive
{

	class EpgInfo;

	typedef std::shared_ptr<EpgInfo> EpgInfoPtr;

	// -------------------------------------------------------------------------

	namespace EpgEvents {

		std::string EncodeDomId(tChannelID const &chanId, tEventID const &eventId);
		void DecodeDomId(cSv epgid, tChannelID &chanId, tEventID &eventId);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed channel and event information.
		 *	Never call this function with a NULL chan pointer
		 */
		EpgInfoPtr CreateEpgInfo(cChannel const *chan, cEvent const *event, cSv idOverride = cSv());

		/**
		 *  This is the inverse creator for epgInfos to the creator above.
		 */
		EpgInfoPtr CreateEpgInfo(cSv epgid, cSchedules const *schedules);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed recording information.
		 */
		EpgInfoPtr CreateEpgInfo(cSv recid, cRecording const *recording, char const *caption = 0);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed string informations
		 */
		EpgInfoPtr CreateEpgInfo(cSv id, cSv caption, cSv info);

		/**
		 *  Return a list of EpgImage paths for a given epgid.
		 */

    bool PosterTvscraper(cTvMedia &media, const cEvent *event, const cRecording *recording);
		std::list<std::string> EpgImages(cSv epgid);

		/**
		 *  Return a list of RecImages in the given folder.
		 */
		std::list<std::string> RecImages(cSv epgid, cSv recfolder);

		/**
		 *  Calculate the duration. A duration can be zero or
		 *  negative. Negative durations are considered invalid by
		 *  LIVE.
		 */
		int Duration(time_t const startTime, time_t const endTime);

		/**
		 *  Calculate the elapsed time of a positive duration. This
		 *  takes into account the startTime and the current time. If
		 *  the current time is not in the interval startTime <=
		 *  currTime <= endTime the return value is -1.
		 */
		int ElapsedTime(time_t const startTime, time_t const endTime);

	} // namespace EpgEvents

	// -------------------------------------------------------------------------

	class EpgInfo
	{
		public:
			EpgInfo(cSv id, cSv caption);
			virtual ~EpgInfo();

			virtual cSv Id() const { return m_eventId; }

			virtual cSv Caption() const { return m_caption; }

			virtual cSv Title() const = 0;

			virtual cSv ShortDescr() const = 0;

			virtual cSv LongDescr() const = 0;

			virtual cChannel const * Channel() const { return 0; }

			virtual cSv ChannelName() const;

			virtual cSv Archived() const { return cSv(); }

			virtual cSv FileName() const { return cSv(); }

			virtual std::string const StartTime(const char* format) const;

			virtual std::string const EndTime(const char* format) const;

			virtual std::string const CurrentTime(const char* format) const;

			virtual int Duration() const;  // for recordings, this is the length of the recording
			virtual int EventDuration() const { return -1; }; // this is always the event duration

			virtual int Elapsed() const;

			virtual time_t GetStartTime() const = 0;

			virtual time_t GetEndTime() const = 0;

			virtual cEvent const *Event() const { return NULL; }
		private:
			std::string m_eventId;
			std::string m_caption;
	};

	// -------------------------------------------------------------------------

	class EpgString : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cSv, cSv, cSv);

		public:
			EpgString(cSv id, cSv caption, cSv info);
			virtual ~EpgString();

			virtual cSv Title() const;

			virtual cSv ShortDescr() const;

			virtual cSv LongDescr() const;

			virtual time_t GetStartTime() const;

			virtual time_t GetEndTime() const;

			virtual cSv FileName() const { return cSv(); }

		private:
			const std::string m_info;
	};

	// -------------------------------------------------------------------------

	class EpgEvent : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cChannel const *, cEvent const *, cSv);

		public:
			EpgEvent(cSv id, cEvent const *event, char const *channelName);
			virtual ~EpgEvent();

			virtual cSv Title() const { return m_event->Title(); }

			virtual cSv ShortDescr() const { return m_event->ShortText(); }

			virtual cSv LongDescr() const { return m_event->Description(); }

			virtual time_t GetStartTime() const { return m_event->StartTime(); }

			virtual time_t GetEndTime() const { return m_event->EndTime(); }

			virtual cChannel const * Channel() const { LOCK_CHANNELS_READ; return Channels->GetByChannelID(m_event->ChannelID());}

			virtual cSv FileName() const { return cSv(); }

			virtual cEvent const *Event() const { return m_event; }
		private:
			cEvent const * m_event;
	};

	// -------------------------------------------------------------------------

	class EmptyEvent : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cChannel const *, cEvent const *, cSv);

		public:
			EmptyEvent(cSv id, tChannelID const &channelID, const char* channelName);
			virtual ~EmptyEvent();

			virtual cSv Title() const { return tr("No EPG information available"); }

			virtual cSv ShortDescr() const { return cSv(); }

			virtual cSv LongDescr() const { return cSv(); }

			virtual time_t GetStartTime() const { return 0; }

			virtual time_t GetEndTime() const { return 0; }

			virtual cChannel const * Channel() const { LOCK_CHANNELS_READ; return Channels->GetByChannelID(m_channelID);}

			virtual cSv FileName() const { return cSv(); }

		private:
			tChannelID m_channelID;
	};

	// -------------------------------------------------------------------------

	class EpgRecording : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cSv, cRecording const *, const char *);

		protected:
			cSv Name() const;

		public:
			EpgRecording(cSv recid, cRecording const *recording, char const *caption);
			virtual ~EpgRecording();

			virtual cSv Caption() const;

			virtual cSv Title() const;

			virtual cSv ShortDescr() const;

			virtual cSv LongDescr() const;

			virtual cSv ChannelName() const;

			virtual cSv Archived() const;

			virtual cSv FileName() const;

			virtual time_t GetStartTime() const;

			virtual time_t GetEndTime() const;
			virtual int EventDuration() const; // this is always the event duration

			virtual int Elapsed() const;

		private:
			const cRecording* m_recording;
			bool m_ownCaption;
			mutable bool m_checkedArchived;
			mutable std::string m_archived;
	};
bool appendEpgItem(cToSvConcat<0> &epg_item, RecordingsItemRecPtr &recItem, const cEvent *Event, const cChannel *Channel, bool withChannel);
}; // namespace vdrlive

#endif // VDR_LIVE_EPG_EVENTS_H
