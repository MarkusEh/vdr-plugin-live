#include "epg_events.h"

#include "tools.h"
#include "timers.h"
#include "recman.h"
#include "setup.h"

// STL headers need to be before VDR tools.h (included by <vdr/player.h>)
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

  EpgInfo::EpgInfo(cSv id, cSv caption):
    m_eventId(id),
    m_caption(caption)
  { }

  EpgInfo::~EpgInfo() { }

  cSv EpgInfo::ChannelName() const
  {  const cChannel* channel = Channel();
    return channel ? channel->Name() : cSv();
  }

  const std::string EpgInfo::CurrentTime(const char* format) const
  {
    return std::string(cToSvDateTime(format, time(0)));
  }

  const std::string EpgInfo::StartTime(const char* format) const
  {
    time_t start = GetStartTime();
    return start ? std::string(cToSvDateTime(format, start)) : "";
  }

  const std::string EpgInfo::EndTime(const char* format) const
  {
    time_t end = GetEndTime();
    return end ? std::string(cToSvDateTime(format, end)) : "";
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

  EpgEvent::EpgEvent(cSv id, const cEvent* event, const char* channelName) :
    EpgInfo(id, channelName),
    m_event(event)
  { }

  EpgEvent::~EpgEvent() { }

  /*
   * -------------------------------------------------------------------------
   * EpgString
   * -------------------------------------------------------------------------
   */

  EpgString::EpgString(cSv id, cSv caption, cSv info) :
    EpgInfo(id, caption),
    m_info(info)
  { }

  EpgString::~EpgString() { }

  cSv EpgString::Title() const
  {
    return m_info;
  }

  cSv EpgString::ShortDescr() const
  {
    return cSv();
  }

  cSv EpgString::LongDescr() const
  {
    return cSv();
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

  EpgRecording::EpgRecording(cSv recid, const cRecording* recording, const char* caption) :
    EpgInfo(recid, caption),
    m_recording(recording),
    m_ownCaption(caption != 0),
    m_checkedArchived(false),
    m_archived()
  { }

  EpgRecording::~EpgRecording()
  {
    m_recording = 0;
  }

  cSv EpgRecording::Caption() const
  {
    if (m_ownCaption) {
      return EpgInfo::Caption();
    }
    if (!m_recording) {
      return cSv();
    }

    return Name();
  }

  cSv EpgRecording::Title() const
  {
    if (!m_recording) {
      return cSv();
    }

    const cRecordingInfo* info = m_recording->Info();
    return (info && info->Title()) ? info->Title() : Name();
  }

  cSv EpgRecording::ShortDescr() const
  {
    const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
    return (info && info->ShortText()) ? info->ShortText() : cSv();
  }

  cSv EpgRecording::LongDescr() const
  {
    const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
    return (info && info->Description()) ? info->Description() : cSv();
  }

  cSv EpgRecording::ChannelName() const
  {
    const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
    return info && info->ChannelName() ? info->ChannelName(): cSv();
  }

  cSv EpgRecording::Archived() const
  {
    if (!m_checkedArchived && m_recording) {
      m_archived = RecordingsManager::GetArchiveDescr(m_recording);
      m_checkedArchived = true;
    }
    return m_archived;
  }

  cSv EpgRecording::FileName() const
  {
    return m_recording->FileName();
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
  int EpgRecording::EventDuration() const
  {
    const cRecordingInfo* info = m_recording ? m_recording->Info() : 0;
    if (!info || !info->GetEvent() ) return 0;
    return info->GetEvent()->Duration();
  }

  int EpgRecording::Elapsed() const
  {
    if (!m_recording)
      return -1;
    int current, total;
    // try currently playing recording if any
#if APIVERSNUM >= 20402
    cMutexLock mutexLock;
    cControl* pControl = cControl::Control(mutexLock);
#else
    cControl* pControl = cControl::Control();
#endif
    if (pControl)
    {
      const cRecording* playing = pControl->GetRecording();
      if (playing && playing->Id() == m_recording->Id()
        && pControl->GetIndex(current,total) && total)
        return (100 * current) / total;
    }
    // Check for resume position next
    current = m_recording->GetResume();
    total= m_recording->NumFrames();
    if (current > 0 && total > 0)
      return (100 * current) / total;
    return -1;
  }

  cSv EpgRecording::Name() const
  {
    cSv name(m_recording->Name());
    size_t index = name.find_last_of('~');
    if (index != std::string::npos) {
      name = name.substr(index+1);
    }
    return name;
  }

  /*
   * -------------------------------------------------------------------------
   * EmptyEvent
   * -------------------------------------------------------------------------
   */

  EmptyEvent::EmptyEvent(cSv id, tChannelID const &channelID, const char* channelName) :
    EpgInfo(id, channelName),
    m_channelID(channelID)
  { }

  EmptyEvent::~EmptyEvent() { }

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

    const cEvent *GetEventByEpgId(cSv epgid) {
      if (epgid.empty() ) return nullptr;
      tChannelID channelid;
      tEventID eventid;
      DecodeDomId(epgid, channelid, eventid);
      if ( !channelid.Valid() || eventid == 0 ) return nullptr;
      LOCK_SCHEDULES_READ;
      const cSchedule *schedule = Schedules->GetSchedule( channelid );
      if (!schedule) return nullptr;
#if APIVERSNUM >= 20502
      return schedule->GetEventById( eventid );
#else
      return schedule->GetEvent( eventid );
#endif
    }

    EpgInfoPtr CreateEpgInfo(cSv epgid, cSchedules const *schedules)
    {
      tEventID eventId = tEventID();
      tChannelID channelId = tChannelID();

      DecodeDomId(epgid, channelId, eventId);
      LOCK_CHANNELS_READ;
      cChannel const *channel = Channels->GetByChannelID(channelId);
      if (!channel) {
        return CreateEpgInfo(epgid, tr("Epg error"), tr("Wrong channel id"));
      }
      cSchedule const *schedule = schedules->GetSchedule(channel);
      if (!schedule) {
        return CreateEpgInfo(epgid, tr("Epg error"), tr("Channel has no schedule"));
      }
#if APIVERSNUM >= 20502
      cEvent const *event = schedule->GetEventById(eventId);
#else
      cEvent const *event = schedule->GetEvent(eventId);
#endif
      if (!event) {
        return CreateEpgInfo(epgid, tr("Epg error"), tr("Wrong event id"));
      }
      return CreateEpgInfo(channel, event, epgid);
    }

    EpgInfoPtr CreateEpgInfo(cChannel const *chan, cEvent const *event, cSv idOverride)
    {
      assert(chan);

      if (event) {
        if (idOverride.empty()) return std::make_shared<EpgEvent>(EncodeDomId(chan->GetChannelID(), event->EventID()), event, chan->Name());
        return std::make_shared<EpgEvent>(idOverride, event, chan->Name());
      }
      if (LiveSetup().GetShowChannelsWithoutEPG()) {
        std::string domId(!idOverride.empty() ? idOverride : EncodeDomId(chan->GetChannelID(), 0));
        return std::make_shared<EmptyEvent>(domId, chan->GetChannelID(), chan->Name());
      }
      return EpgInfoPtr();
    }

    EpgInfoPtr CreateEpgInfo(cSv recid, cRecording const *recording, char const *caption)
    {
      return std::make_shared<EpgRecording>(recid, recording, caption);
    }

    EpgInfoPtr CreateEpgInfo(cSv id, cSv caption, cSv info)
    {
      return std::make_shared<EpgString>(id, caption, info);
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

    bool ScanForRecImages(cSv imageId, cSv recfolder , std::list<std::string> & images)
    {
// format of imageId: <hashed recording file name>
      if (recfolder.empty()) return false;

      bool found = false;
      const std::string filetypes[] = {"png", "jpg", "webp", "PNG", "JPG"};
      int size = sizeof(filetypes)/sizeof(filetypes[0]);

      for (int j = 0; j < size; j++)
      {
        const std::string filemask = concat(recfolder, "/*.", filetypes[j]);
        glob_t globbuf;
        globbuf.gl_offs = 0;
        if (glob(filemask.c_str(), GLOB_DOOFFS, NULL, &globbuf) == 0) {
          for(size_t i = 0; i < globbuf.gl_pathc; i++) {
            const std::string_view imagefile(globbuf.gl_pathv[i]);
            size_t delimPos = imagefile.find_last_of('/');
            const std::string_view imagename(imagefile.substr(delimPos+1));
            images.push_back(std::move(std::string(imagename)));

            // create a temporary symlink of the image in tmpImageDir
            cToSvConcat tmpfile(tmpImageDir, imageId, "_", imagename);
            cToSvConcat imgfile(imagefile);
            imgfile.replaceAll("$", "\\$");
            imgfile.replaceAll("\"", "\\\"");
            tmpfile.replaceAll("$", "\\$");
            tmpfile.replaceAll("\"", "\\\"");
            cToSvConcat cmdBuff("ln -s \"", imgfile, "\" \"", tmpfile, "\"");
            int s = system(cmdBuff.c_str() );
            if (s < 0)
              esyslog("live: ERROR: Couldn't execute command %s", cmdBuff.c_str() );
            found = true;
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

std::string appendEpgItemWithRecItem(cToSvConcat<0> &epg_item, cSv lastDay, const cEvent *Event, const cChannel *Channel, bool withChannel) {
// return current day
  cToSvDateTime day(tr("%A, %b %d %Y"), Event->StartTime());
  if (lastDay != cSv(day)) {
    if (!lastDay.empty()) epg_item.concat("]],\n");
    epg_item.concat("[\"", day, "\",[");
  } else
    epg_item.concat(',');
  RecordingsItemRecPtr recItem;
  epg_item.concat('[');
  if (appendEpgItem(epg_item, recItem, Event, Channel, withChannel)) {
    epg_item.concat(",[");
    recItem->AppendAsJSArray(epg_item);
    epg_item.concat(']');
  }
  epg_item.concat(']');
  return std::string(day);
}

bool appendEpgItem(cToSvConcat<0> &epg_item, RecordingsItemRecPtr &recItem, const cEvent *Event, const cChannel *Channel, bool withChannel) {
  cGetScraperVideo getScraperVideo(Event, nullptr);
  getScraperVideo.call(LiveSetup().GetPluginTvscraper());

  RecordingsTreePtr recordingsTree(LiveRecordingsManager()->GetRecordingsTree());
  const std::vector<RecordingsItemRecPtr> *recItems = recordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
  bool recItemFound = searchNameDesc(recItem, recItems, Event, getScraperVideo.m_scraperVideo.get() );

  epg_item.append("[\"");
// [0] : EPG ID  (without event_)
//  epg_item.append(EpgEvents::EncodeDomId(Channel->GetChannelID(), Event->EventID()).c_str() + 6);
//  epg_item.appendChannel(Channel->GetChannelID(), 'p', 'm');
  stringAppendChannel(epg_item, Channel->GetChannelID(), 'p', 'm');
  epg_item.concat('_', Event->EventID());

  epg_item.append("\",\"");
// [1] : Timer ID
  const cTimer* timer = LiveTimerManager().GetTimer(Event, Channel);
  if (timer) {
    epg_item.append(vdrlive::EncodeDomId(LiveTimerManager().GetTimers().GetTimerId(*timer), ".-:", "pmc"));
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
  epg_item.append(" - ");
  epg_item.appendDateTime(tr("%I:%M %p"), Event->EndTime() );
  epg_item.append(" ");
  AppendDuration(epg_item, tr("(%d:%02d)"), Event->Duration());
  epg_item.append("\"]");
  return recItemFound;
}

}; // namespace vdrlive
