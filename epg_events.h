#ifndef VDR_LIVE_EPG_EVENTS_H
#define VDR_LIVE_EPG_EVENTS_H

#include "stdext.h"

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <string>
#include <list>

#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/recording.h>

namespace vdrlive
{

	class EpgInfo;

	typedef std::tr1::shared_ptr<EpgInfo> EpgInfoPtr;

	// -------------------------------------------------------------------------

	namespace EpgEvents {

		std::string EncodeDomId(tChannelID const &chanId, tEventID const &eventId);
		void DecodeDomId(std::string const &epgid, tChannelID &chanId, tEventID &eventId);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed channel and event information.
		 *	Never call this function with a NULL chan pointer
		 */
		EpgInfoPtr CreateEpgInfo(cChannel const *chan, cEvent const *event, char const *idOverride = 0);

		/**
		 *  This is the inverse creator for epgInfos to the creator above.
		 */
		EpgInfoPtr CreateEpgInfo(std::string const &epgid, cSchedules const *schedules);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed recording information.
		 */
		EpgInfoPtr CreateEpgInfo(std::string const &recid, cRecording const *recording, char const *caption = 0);

		/**
		 *	Allocate and initalize an epgEvent instance with the
		 *	passed string informations
		 */
		EpgInfoPtr CreateEpgInfo(std::string const &id, std::string const &caption, std::string const &info);

		/**
		 *  Return a list of EpgImage paths for a given epgid.
		 */
		std::list<std::string> EpgImages(std::string const &epgid);

		/**
		 *  Return a list of RecImages in the given folder.
		 */
		std::list<std::string> RecImages(std::string const &epgid, std::string const &recfolder);

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
		protected:
			EpgInfo(std::string const &id,
					std::string const &caption);

		public:
			virtual ~EpgInfo();

			virtual std::string const Id() const { return m_eventId; }

			virtual std::string const Caption() const { return m_caption; }

			virtual std::string const Title() const = 0;

			virtual std::string const ShortDescr() const = 0;

			virtual std::string const LongDescr() const = 0;

			virtual cChannel const * Channel() const { return 0; }

			virtual std::string const Archived() const { return ""; }

			virtual std::string const FileName() const { return ""; }

			virtual std::string const StartTime(const char* format) const;

			virtual std::string const EndTime(const char* format) const;

			virtual std::string const CurrentTime(const char* format) const;

			virtual int Duration() const;

			virtual int Elapsed() const;

			virtual time_t GetStartTime() const = 0;

			virtual time_t GetEndTime() const = 0;

		private:
			std::string m_eventId;
			std::string m_caption;
	};

	// -------------------------------------------------------------------------

	class EpgString : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(std::string const &, std::string const &, std::string const &);

		protected:
			EpgString(std::string const &id,
					  std::string const &caption,
					  std::string const &info);

		public:
			virtual ~EpgString();

			virtual std::string const Title() const;

			virtual std::string const ShortDescr() const;

			virtual std::string const LongDescr() const;

			virtual time_t GetStartTime() const;

			virtual time_t GetEndTime() const;

			virtual std::string const FileName() const { return ""; }

		private:
			const std::string m_info;
	};

	// -------------------------------------------------------------------------

	class EpgEvent : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cChannel const *, cEvent const *, char const *);

		protected:
			EpgEvent(std::string const &id,
					 cEvent const *event,
					 char const *channelName);

		public:
			virtual ~EpgEvent();

			virtual std::string const Title() const { return std::string(m_event->Title() ? m_event->Title() : ""); }

			virtual std::string const ShortDescr() const { return std::string(m_event->ShortText() ? m_event->ShortText() : ""); }

			virtual std::string const LongDescr() const { return std::string(m_event->Description() ? m_event->Description() : ""); }

			virtual time_t GetStartTime() const { return m_event->StartTime(); }

			virtual time_t GetEndTime() const { return m_event->EndTime(); }

#if VDRVERSNUM >= 20301
			virtual cChannel const * Channel() const { LOCK_CHANNELS_READ; return Channels->GetByChannelID(m_event->ChannelID());}
#else
			virtual cChannel const * Channel() const { return Channels.GetByChannelID(m_event->ChannelID());}
#endif

			virtual std::string const FileName() const { return ""; }

		private:
			cEvent const * m_event;
	};

	// -------------------------------------------------------------------------

	class EmptyEvent : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(cChannel const *, cEvent const *, char const *);

		protected:
			EmptyEvent(std::string const &id, tChannelID const &channelID, const char* channelName);

		public:
			virtual ~EmptyEvent();

			virtual std::string const Title() const { return tr("No EPG information available"); }

			virtual std::string const ShortDescr() const { return ""; }

			virtual std::string const LongDescr() const { return ""; }

			virtual time_t GetStartTime() const { return 0; }

			virtual time_t GetEndTime() const { return 0; }

#if VDRVERSNUM >= 20301
			virtual cChannel const * Channel() const { LOCK_CHANNELS_READ; return Channels->GetByChannelID(m_channelID);}
#else
			virtual cChannel const * Channel() const { return Channels.GetByChannelID(m_channelID);}
#endif

			virtual std::string const FileName() const { return ""; }

		private:
			tChannelID m_channelID;
	};

	// -------------------------------------------------------------------------

	class EpgRecording : public EpgInfo
	{
		friend EpgInfoPtr EpgEvents::CreateEpgInfo(std::string const &, cRecording const *, char const *);

		protected:
			EpgRecording(std::string const &recid,
						 cRecording const *recording,
						 char const *caption);

			const std::string Name() const;

		public:
			virtual ~EpgRecording();

			virtual std::string const Caption() const;

			virtual std::string const Title() const;

			virtual std::string const ShortDescr() const;

			virtual std::string const LongDescr() const;

			virtual std::string const Archived() const;

			virtual std::string const FileName() const;

			virtual time_t GetStartTime() const;

			virtual time_t GetEndTime() const;

			virtual int Elapsed() const;

		private:
			const cRecording* m_recording;
			bool m_ownCaption;
			mutable bool m_checkedArchived;
			mutable std::string m_archived;
	};

}; // namespace vdrlive

#endif // VDR_LIVE_EPG_EVENTS_H
