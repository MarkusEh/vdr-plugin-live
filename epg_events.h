#ifndef VDR_LIVE_WHATS_ON_H
#define VDR_LIVE_WHATS_ON_H

#include <ctime>
#include <list>

#include <vdr/plugin.h>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/config.h>
#include <vdr/i18n.h>

#include "live.h"
#include "stdext.h"

namespace vdrlive
{

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

			virtual std::string const StartTime(const char* format) const;

			virtual std::string const EndTime(const char* format) const;

			virtual std::string const CurrentTime(const char* format) const;

			virtual int Elapsed() const;

			// virtual const cTimer* GetTimer() const = 0;

			virtual time_t GetStartTime() const = 0;

			virtual time_t GetEndTime() const = 0;

		private:
			std::string m_eventId;
			std::string m_caption;
	};

	typedef std::tr1::shared_ptr<EpgInfo> EpgInfoPtr;

	// -------------------------------------------------------------------------

	class EpgString : public EpgInfo
	{
		friend class EpgEvents;

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

		private:
			const std::string m_info;
	};

	// -------------------------------------------------------------------------

	class EpgEvent : public EpgInfo
	{
		friend class EpgEvents;

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

			virtual cChannel const * Channel() const { return Channels.GetByChannelID(m_event->ChannelID());}

		private:
			cEvent const * m_event;
	};

	// -------------------------------------------------------------------------

	class EpgRecording : public EpgInfo
	{
		friend class EpgEvents;

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

			virtual time_t GetStartTime() const;

			virtual time_t GetEndTime() const;

		private:
			const cRecording* m_recording;
			bool m_ownCaption;
			mutable bool m_checkedArchived;
			mutable std::string m_archived;
	};

	// -------------------------------------------------------------------------

	class EpgEvents {
		public:
			EpgEvents();
			virtual ~EpgEvents();

			static std::string EncodeDomId(tChannelID const &chanId, tEventID const &eventId);
			static void DecodeDomId(std::string const &epgid, tChannelID &chanId, tEventID &eventId);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed channel and event information.
			 */
			static EpgInfoPtr CreateEpgInfo(cChannel const *chan, cEvent const *event, char const *idOverride = 0);

			/**
			 *  This is the inverse creator for epgInfos to the creator above.
			 */
			static EpgInfoPtr CreateEpgInfo(std::string const &epgid, cSchedules const *schedules);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed recording information.
			 */
			static EpgInfoPtr CreateEpgInfo(std::string const &recid, cRecording const *recording, char const *caption = 0);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed string informations
			 */
			static EpgInfoPtr CreateEpgInfo(std::string const &id, std::string const &caption, std::string const &info);

			static std::list<std::string> EpgImages(std::string const &epgid);

			static int ElapsedTime(time_t const startTime, time_t const endTime);

		private:
	};
}; // namespace vdrlive

#endif // VDR_LIVE_WHATS_ON_H

