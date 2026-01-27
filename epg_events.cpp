#include "epg_events.h"

#include "tools.h"
#include "timers.h"
#include "recman.h"
#include "setup.h"

// STL headers need to be before VDR tools.h (included by <vdr/player.h>)
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <cassert>

#include <vdr/plugin.h>
#include <vdr/player.h>

#ifndef TVM2VDR_PL_WORKAROUND
#define TVM2VDR_PL_WORKAROUND  0
#endif


namespace vdrlive
{

  /*
   * -------------------------------------------------------------------------
   * EpgInfo
   * -------------------------------------------------------------------------
   */

  const std::string EpgInfo::CurrentTime(const char* format)
  {
    return std::string(cToSvDateTime(format, time(0)));
  }

  const std::string EpgInfo::StartTime(const char* format) const
  {
    return m_startTime ? std::string(cToSvDateTime(format, m_startTime)) : "";
  }

  const std::string EpgInfo::EndTime(const char* format) const
  {
    return m_endTime ? std::string(cToSvDateTime(format, m_endTime)) : "";
  }

  int EpgInfo::Elapsed() const
  {
    if (m_type == 0) return -1;   //  not an event, not a recording ...
    if (m_type == 1) return EpgEvents::ElapsedTime(m_startTime, m_endTime); // epg event
    if (!m_recordingId) return -1;
    // try currently playing recording if any
#if APIVERSNUM >= 20402
    cMutexLock mutexLock;
    cControl* pControl = cControl::Control(mutexLock);
#else
    cControl* pControl = cControl::Control();
#endif
    if (pControl)
    {
      int current, total;
      LOCK_RECORDINGS_READ;
      const cRecording* playing = pControl->GetRecording();
      if (playing && playing->Id() == m_recordingId
        && pControl->GetIndex(current,total) && total)
        return (100 * current) / total;
    }
    // Check for resume position next
    if (m_resume > 0 && m_numFrames > 0)
      return (100 * m_resume) / m_numFrames;
    return -1;
  }

  cSv EpgInfo::Name() const
  {
    cSv name(m_name);
    size_t index = name.find_last_of('~');
    if (index != std::string::npos) {
      name = name.substr(index+1);
    }
    return name;
  }

  /*
   * -------------------------------------------------------------------------
   * EpgEvents
   * -------------------------------------------------------------------------
   */
  namespace EpgEvents {
    std::string EncodeDomId(tChannelID const &chanId, tEventID eId)
    {
      cToSvConcat eventId("event_");
      stringAppendChannel(eventId, chanId, 'p', 'm').concat('_', eId);
      return std::string(eventId);
    }

    void DecodeDomId(cSv epgid, tChannelID& channelId, tEventID &eventId)
    {
      size_t delimPos = epgid.find_last_of('_');
      cToSvConcat channelIdStr(epgid.substr(6, delimPos - 6));  // remove "event_" at start and "_<eventid>" at end
      vdrlive::DecodeDomId(channelIdStr.begin(), channelIdStr.end(), "mp", "-.");
      channelId = tChannelID::FromString(channelIdStr.c_str());
      eventId = parse_int<tEventID>(epgid.substr(delimPos+1));
    }

    const cEvent *GetEventByEpgId(cSv epgid, const cSchedules *Schedules) {
// LOCK_SCHEDULES_READ -> and pass Schedules.
//    Keep the lock as long as you need the event!
      if (epgid.empty() ) return nullptr;
      tChannelID channelid;
      tEventID eventid;
      DecodeDomId(epgid, channelid, eventid);
      if ( !channelid.Valid() || eventid == 0 ) return nullptr;
      const cSchedule *schedule = Schedules->GetSchedule(channelid);
      if (!schedule) return nullptr;
#if APIVERSNUM >= 20502
      return schedule->GetEventById( eventid );
#else
      return schedule->GetEvent( eventid );
#endif
    }
    bool GetEventChannelByEpgId(const cEvent *&event, const cChannel *&channel, cSv epgid, const cChannels *Channels, const cSchedules *Schedules) {
// LOCK_CHANNELS_READ; LOCK_SCHEDULES_READ -> and pass Channels, Schedules.
//    Keep the lock as long as you need the event!
//    return true if event & channel was found
      event = nullptr;
      channel = nullptr;
      if (epgid.empty() ) return false;
      tChannelID channelid;
      tEventID eventid;
      DecodeDomId(epgid, channelid, eventid);
      if (!channelid.Valid() || eventid == 0) return false;
      channel = Channels->GetByChannelID(channelid);
      if (!channel) return false;
      const cSchedule *schedule = Schedules->GetSchedule(channel);
      if (!schedule) return false;
#if APIVERSNUM >= 20502
      event = schedule->GetEventById(eventid);
#else
      event = schedule->GetEvent(eventid);
#endif
      return event;
    }


    bool ScanForEpgImages(cSv imageId, cSv wildcard, std::list<std::string> & images)
    {
      bool found = false;
      const std::string filemask = concat(LiveSetup().GetEpgImageDir(), "/", imageId, wildcard);
      glob_t globbuf;
      globbuf.gl_offs = 0;
      if (!LiveSetup().GetEpgImageDir().empty() && glob(filemask.c_str(), GLOB_DOOFFS, NULL, &globbuf) == 0) {
        for(size_t i = 0; i < globbuf.gl_pathc; i++) {
          const std::string_view imagefile(globbuf.gl_pathv[i]);
          size_t delimPos = imagefile.find_last_of('/');
          images.push_back(std::move(std::string(imagefile.substr(delimPos+1))));
          found = true;
        }
        globfree(&globbuf);
      }
      return found;
    }
    bool ScanForEpgImages(cSv channelId, cSv eventId, cSv wildcard, std::list<std::string> & images)
    {
      if (LiveSetup().GetEpgImageDir().empty() ) return false;
      if (ScanForEpgImages(cToSvConcat(channelId, "_", eventId), wildcard, images)) return true;
      return ScanForEpgImages(eventId, wildcard, images);
    }

    bool ScanForRecImages(cSv imageId, cSv recfolder, std::list<std::string> & images)
    {
// format of imageId: <hashed recording file name>
      if (recfolder.empty()) return false;

      bool found = false;
      for (cSv filetype: cSplit("png,jpg,jpeg,webp,PNG,JPG", ','))
      {
        cToSvConcat filemask(recfolder, "/*.", filetype);
        glob_t globbuf;
        globbuf.gl_offs = 0;
        if (glob(filemask.c_str(), GLOB_DOOFFS, NULL, &globbuf) == 0) {
          for(size_t i = 0; i < globbuf.gl_pathc; i++) {
            const cSv imagefile(globbuf.gl_pathv[i]);
            size_t delimPos = imagefile.find_last_of('/');
            const cSv imagename(imagefile.substr(delimPos+1));  // file name, part in path after last /

            // create a temporary symlink of the image in tmpImageDir
            cToSvConcat tmpfile(tmpImageDir, imageId, "_", imagename);
            if (symlink(globbuf.gl_pathv[i], tmpfile.c_str()) < 0 && errno != EEXIST) {
              esyslog("live: ERROR: Couldn't create symlink, target = %s, linkpath = %s, error: %s", globbuf.gl_pathv[i], tmpfile.c_str(), strerror(errno));
            } else {
              images.push_back(std::move(std::string(imagename)));
              found = true;
            }
          }
          globfree(&globbuf);
        }
      }
      return found;
    }

    bool PosterTvscraper(cTvMedia &media, const cEvent *event, const cRecording *recording)
    {
      media.path = "";
      media.width = media.height = 0;
      if (LiveSetup().GetTvscraperImageDir().empty() ) return false;
      ScraperGetPoster call;
      call.event = event;
      call.recording = recording;
      if (ScraperCallService("GetPoster", &call) && ! call.poster.path.empty() ) {
        media.path = ScraperImagePath2Live(call.poster.path);
        media.width = call.poster.width;
        media.height = call.poster.height;
        return true;
      }
      return false;
    }

    std::list<std::string> EpgImages(cSv epgid)
    {
// format of epgid: event_<encoded channel>_<epgid>

      size_t delimPosFirst = epgid.find_first_of('_');
      size_t delimPosLast = epgid.find_last_of('_');
      cSv channelIdStr_enc = epgid.substr(delimPosFirst+1, delimPosLast-delimPosFirst-1);
      std::string channelId = vdrlive::DecodeDomId(channelIdStr_enc, "mp", "-.");
      cSv eventId = epgid.substr(delimPosLast+1);


      std::list<std::string> images;

      // Initially we scan for images that follow the scheme
      // '<channelId>_<epgid>_<distinction>.*' where distinction is
      // any character sequence.  Usually distinction will be used
      // to assign more than one image to an epg event. Thus it
      // will be a digit or number.  The sorting of the images
      // will depend on the 'distinction' lexical sorting
      // (similar to what ls does).
      // Example:
      //   S19.2E-1-2222-33333_112123_0.jpg    first epg image for event id 112123
      //   S19.2E-1-2222-33333_112123_1.png    second epg image for event id 112123
      // If no image is found with channelId it will be searched
      // with the eventId only following the scheme:
      // '<eventId>_<distinction>.*'
      // Example:
      //   112123_0.jpg    first epg image for event id 112123
      //   112123_1.png    second epg image for event id 112123
      if (! ScanForEpgImages(channelId, eventId, "_*.*", images))
      {
        // if we didn't find images that follow the scheme
        // above we try to find images that contain only the
        // event id as file name without extension:
        if (! ScanForEpgImages(channelId, eventId, ".*", images))
        {
#if TVM2VDR_PL_WORKAROUND
          // if we didn't get images try to work around a
          // bug in tvm2vdr.  tvm2vdr seems always to use
          // one digit less, which leads in some rare cases
          // to the bug in LIVE, that unrelated and to many
          // images are displayed.  But without this 'fix'
          // no images would be visible at all. The bug
          // should be fixed in tvm2vdr.pl (Perl version of
          // tvm2vdr).  There exists a plugin - also called
          // tvm2vdr - which does not have that bug.
          eventId = eventId.substr(0, eventId.size()-1);
          ScanForEpgImages(channelId, eventId, "*.*", images);
#endif
        }
      }
      return images;
    }

    std::list<std::string> RecImages(cSv epgid, cSv recfolder)
    {
// format of epgid: recording_<hashed recording file name>
      size_t delimPos = epgid.find_last_of('_');
      cSv imageId = epgid.substr(delimPos+1);

      std::list<std::string> images;
      // Scan for all images in recording directory
      ScanForRecImages(imageId, recfolder, images);
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

  void EpgInfo::Clear() {
    *this = EpgInfo();
  }
  void EpgInfo::CreateEpgInfo(cSv epgid)
  {
    const cChannel *channel;
    const cEvent *event;
    LOCK_CHANNELS_READ;
    LOCK_SCHEDULES_READ;
    EpgEvents::GetEventChannelByEpgId(event, channel, epgid, Channels, Schedules);
    if (!channel) {
      CreateEpgInfo(epgid, tr("Epg error"), tr("Wrong channel id"));
      return;
    }
    if (!event) {
      CreateEpgInfo(epgid, tr("Epg error"), tr("Wrong event id"));
      return;
    }
    return CreateEpgInfo(channel, event, epgid);
  }

  void EpgInfo::InitializeScraperVideo(cEvent const *event, cRecording const *recording) {
    cGetScraperVideo getScraperVideo(event, recording);
    if (getScraperVideo.call(LiveSetup().GetPluginTvscraper()))
      m_scraperVideo.swap(getScraperVideo.m_scraperVideo);
  }
  void EpgInfo::CreateEpgInfo(cChannel const *chan, cEvent const *event, cSv idOverride)
  {
    assert(chan);

    if (event) {
      m_type = 1;
      m_eventId = idOverride.empty() ? EpgEvents::EncodeDomId(chan->GetChannelID(), event->EventID()):idOverride;
      m_caption = cSv(chan->Name());
      m_channelId = chan->GetChannelID();
      m_channelName = cSv(chan->Name());
      m_channelNumber = chan->Number();
      m_title = cSv(event->Title());
      m_shortText = cSv(event->ShortText());
      m_description = cSv(event->Description());
      m_startTime = event->StartTime();
      m_endTime = event->EndTime();
      m_eventDuration = event->Duration();
      InitializeScraperVideo(event, nullptr);
      for (int i = 0; i < MaxEventContents; ++i) m_contents[i] = event->Contents(i);
      m_parentalRating = event->ParentalRating();
      return;
    }
    if (LiveSetup().GetShowChannelsWithoutEPG()) {
      m_type = 1;
      m_eventId = idOverride.empty() ? EpgEvents::EncodeDomId(chan->GetChannelID(), 0):idOverride;
      m_caption = cSv(chan->Name());
      m_channelId = chan->GetChannelID();
      m_channelName = cSv(chan->Name());
      m_channelNumber = chan->Number();
    }
  }

  void EpgInfo::CreateEpgInfo(cSv recid, cRecording const *recording, char const *caption)
  {
    m_eventId = recid;
    m_type = 2;
    for (int i = 0; i < MaxEventContents; ++i) m_contents[i] = 0;
    if (recording) {
      m_recordingId = recording->Id();
      m_fileName = cSv(recording->FileName());
      m_name = cSv(recording->Name());
      const cRecordingInfo* info = recording->Info();
      if (info) {
        m_title = cSv(info->Title());
        m_shortText = cSv(info->ShortText());
        m_description = cSv(info->Description());
        m_channelId = info->ChannelID();
        m_channelName = cSv(info->ChannelName());
        const cEvent *event = info->GetEvent();
        if (event) {
          m_eventDuration = event->Duration();
          for (int i = 0; i < MaxEventContents; ++i) m_contents[i] = event->Contents(i);
          m_parentalRating = event->ParentalRating();
        }
      }
      if (m_title.empty()) m_title = Name();
      m_startTime = recording->Start();
      int length = recording->LengthInSeconds();
      m_endTime = (length < 0) ? m_startTime : m_startTime + length;
      m_resume = recording->GetResume();
      m_numFrames = recording->NumFrames();
      m_archived = RecordingsManager::GetArchiveDescr(recording);
      InitializeScraperVideo(nullptr, recording);
    }

    if (caption) {
      m_caption = caption;
    } else if (recording) {
      m_caption = Name();
    }
  }

  void EpgInfo::CreateEpgInfo(cSv id, cSv caption, cSv info) {
    m_eventId = id;
    m_caption = caption;
    m_title = info;
  }

void AppendScraperData(cToSvConcat<0> &target, cScraperVideo *scraperVideo) {
  cTvMedia s_image;
  std::string s_title, s_episode_name, s_IMDB_ID, s_release_date;
  if (scraperVideo == NULL) {
    AppendScraperData(target, s_IMDB_ID, s_image, tNone, s_title, 0, 0, s_episode_name, 0, s_release_date);
    return;
  }
  int s_runtime;
  scraperVideo->getOverview(&s_title, &s_episode_name, &s_release_date, &s_runtime, &s_IMDB_ID, NULL);
  s_image = scraperVideo->getImage(
    cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection),
    cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner), false);
  AppendScraperData(target, s_IMDB_ID, s_image, scraperVideo->getVideoType(), s_title, scraperVideo->getSeasonNumber(), scraperVideo->getEpisodeNumber(), s_episode_name, s_runtime, s_release_date);
}


// first call with lastDay = ""
// before first call: open with "["
// after last call: cloes with "]]]"
// [
//   ["Day1", [
//             [[event1], [rec1]], [[event2]], [[event3]]
//            ]
//   ],
//   ["Day2", []
//   ]
// ]

std::string appendEpgItemWithRecItem(cToSvConcat<0> &epg_item, cSv lastDay, const cEvent *Event, const cChannel *Channel, bool withChannel, const cTimers *Timers) {
// return current day
  cToSvDateTime day(tr("%A, %b %d %Y"), Event->StartTime());
  if (lastDay != cSv(day)) {
    if (!lastDay.empty()) epg_item.concat("]],\n");
    epg_item.concat("[\"", day, "\",[");
  } else
    epg_item.concat(',');
  RecordingsItemRec *recItem;
  epg_item.concat('[');
  if (appendEpgItem(epg_item, recItem, Event, Channel, withChannel, Timers)) {
    epg_item.concat(",[");
    recItem->AppendAsJSArray(epg_item);
    epg_item.concat(']');
  }
  epg_item.concat(']');
  return std::string(day);
}

bool appendEpgItem(cToSvConcat<0> &epg_item, RecordingsItemRec *&recItem, const cEvent *Event, const cChannel *Channel, bool withChannel, const cTimers *Timers) {
  cGetScraperVideo getScraperVideo(Event, nullptr);
  getScraperVideo.call(LiveSetup().GetPluginTvscraper());

  RecordingsTreePtr recordingsTree(RecordingsManager::GetRecordingsTree());
  const std::vector<RecordingsItemRec *> *recItems = recordingsTree->allRecordings(RecordingsManager::eSortOrder::duplicatesLanguage, 0);
  bool recItemFound = searchNameDesc(recItem, recItems, Event, getScraperVideo.m_scraperVideo.get());

  epg_item.append("[\"");
// [0] : EPG ID  (without event_)
//  epg_item.append(EpgEvents::EncodeDomId(Channel->GetChannelID(), Event->EventID()).c_str() + 6);
  stringAppendChannel(epg_item, Channel->GetChannelID(), 'p', 'm');
  epg_item.concat('_', Event->EventID());

  epg_item.append("\",\"");
// [1] : Timer ID
  const cTimer* timer = TimerManager::GetTimer(Event, Channel, Timers);
  if (timer) {
    epg_item.append(vdrlive::EncodeDomId(SortedTimers::GetTimerId(*timer), ".-:", "pmc"));
    if (timer->Recording()) {
      epg_item.append("&ts=r");
      // do not show a recording that is underway
      recItemFound = false;
    } else if (!(timer->Flags() & tfActive))
      epg_item.append("&ts=i");
  }
  epg_item.append("\",");
// scraper data
  AppendScraperData(epg_item, getScraperVideo.m_scraperVideo.get() );
  epg_item.append(",");
// [9] : channelnr
  if (withChannel) {
    epg_item.concat(Channel->Number());
    epg_item.append(",\"");
// [10] : channelname
    AppendHtmlEscapedAndCorrectNonUTF8(epg_item, Channel->Name() );
  } else epg_item.append("0,\"");
  epg_item.append("\",\"");
// [11] : Name
  AppendQuoteEscapedAndCorrectNonUTF8(epg_item, Event->Title() );
  epg_item.append("\",\"");
// [12] : Shorttext
  AppendHtmlEscapedAndCorrectNonUTF8(epg_item, Event->ShortText() );
  epg_item.append("\",\"");
// [13] : Description
  AppendTextTruncateOnWord(epg_item, Event->Description(), LiveSetup().GetMaxTooltipChars(), true);
  epg_item.append("\",\"");
// [14] : Day, time & duration of event
  epg_item.appendDateTime(tr("%I:%M %p"), Event->StartTime() );
  epg_item.append(tr(" - "));
  epg_item.appendDateTime(tr("%I:%M %p"), Event->EndTime() );
  epg_item.append(" ");
  AppendDuration(epg_item, tr("(%d:%02d)"), Event->Duration());
  epg_item.append("\"]");
  return recItemFound;
}

}; // namespace vdrlive
