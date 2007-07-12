#ifndef VDR_LIVE_WHATS_ON_H
#define VDR_LIVE_WHATS_ON_H

#include <ctime>

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
			EpgInfo(const std::string& id,
					const std::string& caption);

		public:
			virtual ~EpgInfo();

			virtual const std::string Id() const { return m_eventId; }

			virtual const std::string Caption() const { return m_caption; }

			virtual const std::string Title() const = 0;

			virtual const std::string ShortDescr() const = 0;

			virtual const std::string LongDescr() const = 0;

			virtual const std::string Archived() const { return ""; }

			virtual const std::string StartTime(const char* format) const;

			virtual const std::string EndTime(const char* format) const;

			virtual const std::string CurrentTime(const char* format) const;

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
			EpgString(const std::string& id,
					  const std::string& caption,
					  const std::string& info);

		public:
			virtual ~EpgString();

			virtual const std::string Title() const;

			virtual const std::string ShortDescr() const;

			virtual const std::string LongDescr() const;

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
			EpgEvent(const std::string& id,
					 const cEvent* event,
					 const char* channelName = "");

		public:
			virtual ~EpgEvent();

			virtual const std::string Title() const { return std::string(m_event->Title() ? m_event->Title() : ""); }

			virtual const std::string ShortDescr() const { return std::string(m_event->ShortText() ? m_event->ShortText() : ""); }

			virtual const std::string LongDescr() const { return std::string(m_event->Description() ? m_event->Description() : ""); }

			virtual time_t GetStartTime() const { return m_event->StartTime(); }

			virtual time_t GetEndTime() const { return m_event->EndTime(); }

		private:
			const cEvent* m_event;
	};

	// -------------------------------------------------------------------------

	class EpgRecording : public EpgInfo
	{
		friend class EpgEvents;

		protected:
			EpgRecording(const std::string& recid, const cRecording* recording, const char* caption);

			const std::string Name() const;

		public:
			virtual ~EpgRecording();

			virtual const std::string Caption() const;

			virtual const std::string Title() const;

			virtual const std::string ShortDescr() const;

			virtual const std::string LongDescr() const;

			virtual const std::string Archived() const;

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

			static std::string GetDomId(const tChannelID& chanId, const tEventID& eventId);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed channel and event information.
			 */
			static EpgInfoPtr CreateEpgInfo(const cChannel* chan, const cEvent* event, const char* idOverride = 0);

			/**
			 *  This is the inverse creator for epgInfos to the creator above.
			 */
			static EpgInfoPtr CreateEpgInfo(const std::string& epgid, const cSchedules* schedules);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed recording information.
			 */
			static EpgInfoPtr CreateEpgInfo(const std::string& recid, const cRecording* recording, const char* caption = 0);

			/**
			 *	Allocate and initalize an epgEvent instance with the
			 *	passed string informations
			 */
			static EpgInfoPtr CreateEpgInfo(const std::string& id, const std::string& caption, const std::string& info);
		private:
	};
}; // namespace vdrlive

#endif // VDR_LIVE_WHATS_ON_H

