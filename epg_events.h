#ifndef VDR_LIVE_EPG_EVENTS_H
#define VDR_LIVE_EPG_EVENTS_H

#include "stdext.h"

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <string>
#include <list>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include "services.h"
#include "recman.h"
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/recording.h>

namespace vdrlive
{

  class EpgInfo;

  typedef EpgInfo* EpgInfoPtr;

  // -------------------------------------------------------------------------

  namespace EpgEvents {

    std::string EncodeDomId(tChannelID const &chanId, tEventID eventId);
    void DecodeDomId(cSv epgid, tChannelID &chanId, tEventID &eventId);

    const cEvent *GetEventByEpgId(cSv epgid, const cSchedules *Schedules);
    bool GetEventChannelByEpgId(const cEvent *&event, const cChannel *&channel, cSv epgid, const cChannels *Channels, const cSchedules *Schedules);


    bool PosterTvscraper(cTvMedia &media, const cEvent *event, const cRecording *recording);
    /**
     *  Return a list of EpgImage paths for a given epgid.
     */
    std::list<std::string> EpgImages(cSv epgid);

    /**
     *  Return a list of RecImages in the given folder.
     */
    std::list<std::string> RecImages(cSv epgid, cSv recfolder);

    /**
     *  Calculate the duration. A duration can be zero or
     *  positive. Negative durations are considered invalid by
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
      EpgInfo() { m_contents.fill(uchar(0)); }
      EpgInfo(cChannel const *chan, cEvent const *event) {
        m_contents.fill(uchar(0));
        CreateEpgInfo(chan, event);
      }
      void Clear();
      /**
       *  Allocate and initialize an epgEvent instance with the
       *  passed channel and event information.
       *  Never call this function with a NULL chan pointer
       */
      void CreateEpgInfo(cChannel const *chan, cEvent const *event, cSv idOverride = cSv());

      /**
       *  This is the inverse creator for epgInfos to the creator above.
       */
      void CreateEpgInfo(cSv epgid);

      /**
       *  Allocate and initialize an epgEvent instance with the
       *  passed recording information.
       */
      void CreateEpgInfo(cSv recid, cRecording const *recording, char const *caption = 0);

      /**
       *  Allocate and initialize an epgEvent instance with the
       *  passed string informations
       */
      void CreateEpgInfo(cSv id, cSv caption, cSv info);

      bool isRecording() const { return m_type == 2;}
      cSv Id() const { return m_eventId; }
      cSv Caption() const { return m_caption; }
      cSv Title() const { return m_title; }
      cSv ShortDescr() { return m_shortText; }
      cSv LongDescr() { return m_description; }
      cSv ChannelName() { return m_channelName; }
      int ChannelNumber() { return m_channelNumber; }
      std::string const StartTime(const char* format) const;
      std::string const EndTime(const char* format) const;
      static std::string const CurrentTime(const char* format);
      time_t GetStartTime() const { return m_startTime; }
      time_t GetEndTime() const { return m_endTime; }
      int Duration() const { return m_endTime-m_startTime; }  // for recordings: recording duration
      int EventDuration() const { return m_eventDuration; } // this is always the event duration
      int Elapsed() const;
      uchar Contents(int i = 0) const { return (0 <= i && i < MaxEventContents) ? m_contents[i] : uchar(0); }
      int ParentalRating() const { return m_parentalRating; }
      bool ScraperVideoAvailable() const { return bool(m_scraperVideo) ; }
      cScraperVideo *GetScraperVideo() const { return m_scraperVideo.get() ; }
// only for recordings:
      cSv Archived() const { return m_archived; }
      cSv FileName() const { return m_fileName; }
    private:
      int m_type = 0; // o -> none, 1 -> event, 2 -> recording
      std::string m_eventId;
      std::string m_caption;
      std::string m_title;
      std::string m_shortText;
      std::string m_description;
      time_t m_startTime = 0;
      time_t m_endTime = 0;
      tChannelID m_channelId = tChannelID();
      std::string m_channelName;
      int m_channelNumber = 0;
      int m_eventDuration = 0;  // this is always the event duration
      std::array<uchar, MaxEventContents> m_contents;
      int m_parentalRating = 0;
      void InitializeScraperVideo(cEvent const *event, cRecording const *recording);
      std::unique_ptr<cScraperVideo> m_scraperVideo;
// for recordings:
      int m_recordingId = 0;
      int m_resume = 0;
      int m_numFrames = 0;
      std::string m_archived;
      std::string m_fileName;
      std::string m_name;  // as returned by cRecording:Name()
      cSv Name() const; // Last Part of m_name
  };

  // -------------------------------------------------------------------------

bool appendEpgItem(cToSvConcat<0> &epg_item, RecordingsItemRecPtr &recItem, const cEvent *Event, const cChannel *Channel, bool withChannel, const cTimers *Timers);
std::string appendEpgItemWithRecItem(cToSvConcat<0> &epg_item, cSv lastDay, const cEvent *Event, const cChannel *Channel, bool withChannel, const cTimers *Timers);
}; // namespace vdrlive

#endif // VDR_LIVE_EPG_EVENTS_H
