#include "timers.h"

#include "exception.h"
#include "tools.h"

// STL headers need to be before VDR tools.h (included by <vdr/plugin.h>)
#include <sstream>
#include <memory>

#include <vdr/plugin.h>
#include <vdr/menu.h>
#include <vdr/svdrp.h>
#include "services.h"
#include "setup.h"

namespace vdrlive {

  static char const* const TIMER_DELETE = "DELETE";
  static char const* const TIMER_TOGGLE = "TOGGLE";

  std::string SortedTimers::GetTimerId(cTimer const& timer)
  {
    return std::string(cToSvConcat(timer.Channel()->GetChannelID(), ':', timer.WeekDays(), ':', timer.Day(), ':', timer.Start(), ':', timer.Stop()) );
  }

  const cTimer* SortedTimers::GetByTimerId(cSv timerid, const cTimers* Timers)
  {
    cSplit parts(timerid, ':');
    if (parts.size() < 5) {
      esyslog("live: GetByTimerId: invalid format %.*s", (int)timerid.length(), timerid.data() );
      return nullptr;
    }

#ifdef DEBUG_LOCK
    dsyslog("live: timers.cpp SortedTimers::GetByTimerId() LOCK_CHANNELS_READ");
#endif
    LOCK_CHANNELS_READ;
    auto part = parts.begin();
    const cChannel* channel = Channels->GetByChannelID(lexical_cast<tChannelID>(*part, tChannelID(), "SortedTimers::GetByTimerId"));
    if (!channel) {
      esyslog("live: GetByTimerId: no channel %.*s", (int)(*part).length(), (*part).data() );
      return nullptr;
    }

    int weekdays = parse_int<int>(*++part);
    time_t day = parse_int<time_t>(*++part);
    int start = parse_int<int>(*++part);
    int stop = parse_int<int>(*++part);

    for (const cTimer* timer = Timers->First(); timer; timer = Timers->Next(timer)) {
      if ( timer->Channel() == channel &&
         ( ( weekdays != 0 && timer->WeekDays() == weekdays ) || ( weekdays == 0 && timer->Day() == day ) ) &&
         timer->Start() == start && timer->Stop() == stop )
        return timer;
    }
    return nullptr;
  }

  std::string SortedTimers::EncodeDomId(cSv timerid)
  {
    cToSvConcat tId("timer_");
    size_t enc_begin = tId.length();
    tId.append(timerid);
    vdrlive::EncodeDomId(tId.begin() + enc_begin, tId.end(), ".-:", "pmc");
    return std::string(tId);
  }

  std::string SortedTimers::DecodeDomId(cSv timerDomId)
  {
    cSv timerStr("timer_");
    cToSvConcat tId(timerDomId.substr(timerStr.length()));
    vdrlive::DecodeDomId(tId.begin(), tId.end(), "pmc", ".-:");
    return std::string(tId);
  }

  std::string SortedTimers::GetTimerDays(cTimer const *timer)
  {
    if (!timer) return "";
    std::string currentDay = timer->WeekDays() > 0 ?
      *cTimer::PrintDay(0, timer->WeekDays(), false) :
      std::string(cToSvDateTime(tr("%A, %x"), timer->Day()));
    return currentDay;
  }

  std::string SortedTimers::GetTimerInfo(cTimer const& timer)
  {
    std::stringstream info;
    info << trVDR("Priority") << ": " << timer.Priority() << std::endl;
    info << trVDR("Lifetime") << ": " << timer.Lifetime() << std::endl;
    info << trVDR("VPS") << ": " << (timer.HasFlags(tfVps)?trVDR("yes"):trVDR("no")) << std::endl;

    if (timer.Aux())
    {
      cSv epgsearchinfo = partInXmlTag(timer.Aux(), "epgsearch");
      if (!epgsearchinfo.empty())
      {
        cSv searchtimer = partInXmlTag(epgsearchinfo, "searchtimer");
        if (!searchtimer.empty())
          info << tr("Search timer") << ": " << searchtimer << std::endl;
      }
    }
    if (!timer.Local()) {
      info << trVDR("Record on") << ": " << timer.Remote() << std::endl;
    }
    return info.str();
  }

  std::string SortedTimers::TvScraperTimerInfo(cTimer const& timer, std::string &recID, std::string &recName) {
    if (!timer.Aux()) return "";
    cGetAutoTimerReason getAutoTimerReason;
    getAutoTimerReason.timer = &timer;
    getAutoTimerReason.requestRecording = true;
    {
      LOCK_RECORDINGS_READ;
      if (getAutoTimerReason.call(LiveSetup().GetPluginTvscraper()) ) {
        if (!getAutoTimerReason.createdByTvscraper) return "";
        if (getAutoTimerReason.recording) {
          recID = concat("recording_", cToSvXxHash128(getAutoTimerReason.recording->FileName() ));
          recName = std::move(getAutoTimerReason.recordingName);
          return std::move(getAutoTimerReason.reason);
        }
        return concat(getAutoTimerReason.reason, " ", getAutoTimerReason.recordingName);
      }
    }
// fallback information, if this Tvscraper method is not available
    cSv tvScraperInfo = partInXmlTag(timer.Aux(), "tvscraper");
    if (tvScraperInfo.empty()) return "";
    cSv data = partInXmlTag(tvScraperInfo, "reason");
    if (data.empty() ) return "";
    return concat(data, " ", partInXmlTag(tvScraperInfo, "causedBy"));
  }

  void TimerManager::UpdateTimer( int timerId, const char* remote, const char* oldRemote, const tChannelID& channel, cStr builder)
  {
    if (remote && !*remote) remote = nullptr;
    if (oldRemote && !*oldRemote) oldRemote = nullptr;
    dsyslog("live: UpdateTimer() timerId '%d'", timerId);
    dsyslog("live: UpdateTimer() remote '%s'", remote);
    dsyslog("live: UpdateTimer() oldRemote '%s'", oldRemote);
    dsyslog("live: UpdateTimer() channel '%s'", *(channel.ToString()));
    dsyslog("live: UpdateTimer() channel '%s'", cToSvConcat(channel).c_str() );
    dsyslog("live: UpdateTimer() builder '%s'", builder.c_str());

    timerStruct timerData = { .id = timerId, .remote=remote, .oldRemote=oldRemote, .builder=std::string(builder) };

    // dsyslog("live: SV: in UpdateTimer");
    m_updateTimers.push_back( timerData );
    // dsyslog("live: SV: wait for update");
    DoUpdateTimers();
    // dsyslog("live: SV: update done");

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::DelTimer( int timerId, const char* remote )
  {
    dsyslog("live: DelTimer() timerId '%d'", timerId);
    dsyslog("live: DelTimer() remote '%s'", remote);

    timerStruct timerData{ .id=timerId, .remote=remote, .oldRemote=remote, .builder=TIMER_DELETE };

    m_updateTimers.push_back( timerData );
    DoUpdateTimers();

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::ToggleTimerActive( int timerId, const char* remote)
  {
    timerStruct timerData{ .id=timerId, .remote=remote, .oldRemote=remote, .builder=TIMER_TOGGLE };

    m_updateTimers.push_back( timerData );
    DoUpdateTimers();

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::DoUpdateTimers()
  {
    // dsyslog("live: SV: updating timers");
    for ( TimerList::iterator timer = m_updateTimers.begin(); timer != m_updateTimers.end(); ++timer ) {
//      dsyslog("live: DoUpdateTimers() timerid '%d'", timer->id );
//      dsyslog("live: DoUpdateTimers() remote '%s'", timer->remote );
//      dsyslog("live: DoUpdateTimers() builder '%s'", timer->builder.c_str() );
      if ( timer->id == 0 ) // new timer
        DoInsertTimer( *timer );
      else if ( timer->builder == TIMER_DELETE ) // delete timer
        DoDeleteTimer( *timer );
      else if ( timer->builder == TIMER_TOGGLE ) // toggle timer
        DoToggleTimer( *timer );
      else // update timer
        DoUpdateTimer( *timer );
    }
    m_updateTimers.clear();
  }

  void TimerManager::DoInsertTimer( timerStruct& timerData )
  {
    if ( timerData.remote ) {  // add remote timer via svdrpsend
      dsyslog("live: DoInsertTimer() add remote timer on server '%s'", timerData.remote);
      cStringList response;
      std::string command = "NEWT ";
      command.append(timerData.builder);
      dsyslog("live: DoInsertTimer() svdrp command '%s'", command.c_str());
      bool svdrpOK = ExecSVDRPCommand(timerData.remote, command.c_str(), &response);
      if ( !svdrpOK ) {
        esyslog("live: svdrp command on remote server %s failed", timerData.remote);
      }
      else {
           for (int i = 0; i < response.Size(); i++) {
                int code = SVDRPCode(response[i]);
                if (code != 250) {
                  esyslog("live: DoInsertTimer() svdrp failed, response: %s", response[i]);
            svdrpOK = false;
                                  }
           }
        if ( svdrpOK ) {
          isyslog("live: remote timer '%s' on server '%s' added", timerData.builder.c_str(), timerData.remote);
        }
        else {
          dsyslog("live: TimerManager::DoInsertTimer(): error in settings for remote timer");
                StoreError(timerData, tr("Error in timer settings"));
        }
      }
      response.Clear();
    }
    else {        // add local timer
      std::unique_ptr<cTimer> newTimer( new cTimer );
      if ( !newTimer->Parse( timerData.builder.c_str() ) ) {
        dsyslog("live: TimerManager::DoInsertTimer(): error in settings for local timer");
        StoreError( timerData, tr("Error in timer settings") );
        return;
      }
      dsyslog("live: DoInsertTimer() add local timer");
#ifdef DEBUG_LOCK
      dsyslog("live: timers.cpp TimerManager::DoInsertTimer() LOCK_TIMERS_WRITE");
#endif
      LOCK_TIMERS_WRITE;
      Timers->SetExplicitModify();
      const cTimer *checkTimer = Timers->GetTimer( newTimer.get() );
      if (checkTimer) {
        StoreError( timerData, tr("Timer already defined") );
        return;
      }
      Timers->Add( newTimer.get() );
      Timers->SetModified();
      isyslog( "live: local timer %s added", *newTimer->ToDescr() );
      newTimer.release();
    }
  }

  void TimerManager::DoUpdateTimer( timerStruct& timerData )
  {
    dsyslog("live: DoUpdateTimer() timerid '%d'", timerData.id );
    dsyslog("live: DoUpdateTimer() remote '%s'", timerData.remote );
    dsyslog("live: DoUpdateTimer() oldRemote '%s'", timerData.oldRemote );
    dsyslog("live: DoUpdateTimer() builder '%s'", timerData.builder.c_str() );

    if ( timerData.remote && timerData.oldRemote ) {  // old and new are remote
      if ( timerData.remote == timerData.oldRemote ) {  // timer stays on the same remote server
        dsyslog("live: DoUptimer() update timer on remote server '%s'", timerData.remote);
        cStringList response;
        std::string command = "MODT ";
        command.append(cToSvInt(timerData.id));
        command.append(" ");
        command.append(timerData.builder);
        dsyslog("live: DoUpdateTimer() svdrp command '%s'", command.c_str());
        bool svdrpOK = ExecSVDRPCommand(timerData.remote, command.c_str(), &response);
        if ( !svdrpOK ) {
          esyslog("live: svdr command on remote server %s failed", timerData.remote);
        }
        else {
          bool responseOK = true;
          for (int i = 0; i < response.Size(); i++) {
            int code = SVDRPCode(response[i]);
            if (code != 250) {
              esyslog("live: DoInsertTimer() svdrp response: %s", response[i]);
              responseOK = false;
            }
          }
          if ( responseOK ) {
            isyslog("live: remote timer '%s' on server '%s' updated", command.c_str(), timerData.remote);
          }
          else {
            StoreError(timerData, tr("Error in timer settings"));
          }
        }
        response.Clear();
      }
      else {      // timer moves from one remote server to another
        isyslog("live: DoUptimer() move timer from remote server '%s' to remote server '%s'", timerData.oldRemote, timerData.remote);
        timerStruct timerDataMove = { .id = timerData.id, .remote=timerData.oldRemote, .oldRemote=timerData.oldRemote, .builder=timerData.builder};
        DoDeleteTimer( timerDataMove );
        timerDataMove = { .id = timerData.id, .remote=timerData.remote, .oldRemote=timerData.remote, .builder=timerData.builder};
        DoInsertTimer( timerDataMove );
      }

    }
    else if ( timerData.remote && !timerData.oldRemote ) {    // move timer from local to remote
      dsyslog("live: DoUpdateTimer() move timer from local to remote server");
      timerStruct timerDataMove = { .id = timerData.id, .remote=NULL, .oldRemote=NULL, .builder=timerData.builder};
      DoDeleteTimer( timerDataMove );
      timerDataMove = { .id = timerData.id, .remote=timerData.remote, .oldRemote=timerData.remote, .builder=timerData.builder};
      DoInsertTimer( timerDataMove );
    }
    else if ( !timerData.remote && timerData.oldRemote ) {    // move timer from remote to local
      dsyslog("live: DoUpdateTimer() move timer from remote server to local");
      timerStruct timerDataMove = { .id = timerData.id, .remote=timerData.oldRemote, .oldRemote=timerData.oldRemote, .builder=timerData.builder};
      DoDeleteTimer( timerDataMove );
      timerDataMove = { .id = timerData.id, .remote=NULL, .oldRemote=NULL, .builder=timerData.builder};
      DoInsertTimer( timerDataMove );
    }
    else {        // old and new are local
      dsyslog("live: DoUpdateTimer() old and new timer are local");
#ifdef DEBUG_LOCK
      dsyslog("live: timers.cpp TimerManager::DoUpdateTimer() LOCK_TIMERS_WRITE");
#endif
      LOCK_TIMERS_WRITE;
      Timers->SetExplicitModify();
      cTimer* oldTimer = Timers->GetById( timerData.id, timerData.oldRemote );
      dsyslog("live: DoUpdateTimer() change local timer '%s'", *oldTimer->ToDescr());
      if ( oldTimer == 0 ) {
        StoreError( timerData, tr("Timer not defined") );
      return;
      }

      cTimer copy = *oldTimer;
      dsyslog("live: old timer flags: %u", copy.Flags());
      if ( !copy.Parse( timerData.builder.c_str() ) ) {
        StoreError( timerData, tr("Error in timer settings") );
        return;
      }
      if (oldTimer->HasFlags(tfRecording)) copy.SetFlags(tfRecording);  // changed a running recording, restore flag "tfRecording"
      dsyslog("live: new timer flags: %u", copy.Flags());
      *oldTimer = copy;

      Timers->SetModified();
      isyslog("live: local timer %s modified (%s)", *oldTimer->ToDescr(), oldTimer->HasFlags(tfActive) ? "active" : "inactive");
    }
  }

  void TimerManager::DoDeleteTimer( timerStruct& timerData )
  {
    dsyslog("live: DoDeleteTimer() timerid '%d'", timerData.id );
    dsyslog("live: DoDeleteTimer() remote '%s'", timerData.remote );
    dsyslog("live: DoDeleteTimer() builder '%s'", timerData.builder.c_str() );

    if ( timerData.remote ) {    // delete remote timer via svdrpsend
      dsyslog("live: DoDeleteTimer() delete remote timer id '%d' from server '%s'", timerData.id, timerData.remote);
      cStringList response;
      std::string command = "DELT ";
      command.append(cToSvInt(timerData.id));
      bool svdrpOK = ExecSVDRPCommand(timerData.remote, command.c_str(), &response);
      if ( !svdrpOK ) {
        esyslog( "live: delete remote timer id %d failed", timerData.id);
      }
      else {
        for (int i = 0; i < response.Size(); i++) {
          int code = SVDRPCode(response[i]);
          if (code != 250) {
            esyslog("live: DoDeleteTimer() svdrp failed, response: %s", response[i]);
            svdrpOK = false;
          }
        }
        if ( svdrpOK ) {
          isyslog("live: remote timer '%s' on server '%s' deleted", command.c_str(), timerData.remote);
        }
        else {
          StoreError(timerData, tr("Error in timer settings"));
        }
      }
      response.Clear();
    }
    else {          // delete local timer
      dsyslog("live: DoDeleteTimer() delete local timer id '%d'", timerData.id);

#ifdef DEBUG_LOCK
      dsyslog("live: timers.cpp TimerManager::DoDeleteTimer() LOCK_TIMERS_WRITE");
#endif
      LOCK_TIMERS_WRITE;
      Timers->SetExplicitModify();
      cTimer* oldTimer = Timers->GetById( timerData.id,  timerData.remote );
      if ( oldTimer == 0 ) {
        StoreError( timerData, tr("Timer not defined") );
        return;
         }
      cTimer copy = *oldTimer;
      if ( oldTimer->Recording() ) {
        oldTimer->Skip();
        cRecordControls::Process( Timers, time( 0 ) );
         }
      Timers->Del( oldTimer );
      Timers->SetModified();
      isyslog("live: local timer %s deleted", *copy.ToDescr());
    }
  }

  void TimerManager::DoToggleTimer( timerStruct& timerData )
  {
    if ( timerData.remote ) {    // toggle remote timer via svdrpsend
#ifdef DEBUG_LOCK
      dsyslog("live: timers.cpp TimerManager::DoToggleTimer() LOCK_TIMERS_READ");
#endif
      LOCK_TIMERS_READ;
      const cTimer* toggleTimer = Timers->GetById( timerData.id, timerData.remote );
      std::string command = "MODT ";
      command.append(cToSvInt(timerData.id));
      if (toggleTimer->HasFlags(tfActive)) {
        dsyslog("live: DoToggleTimer() timer is active");
        command.append(" off");
      }
      else {
        dsyslog("live: DoToggleTimer() timer is not active");
        command.append(" on");
      }
      cStringList response;
      dsyslog("live: DoToggleTimer svdrp command '%s'", command.c_str());
      bool svdrpOK = ExecSVDRPCommand(timerData.remote, command.c_str(), &response);
      if ( !svdrpOK ) {
        esyslog("live: svdr command on remote server %s failed", timerData.remote);
      }
      else {
        for (int i = 0; i < response.Size(); i++) {
                                        int code = SVDRPCode(response[i]);
          if (code != 250) {
            esyslog("live: DoToggleTimer() svdrp failed, response: %s", response[i]);
            svdrpOK = false;
          }
        }
        if ( svdrpOK ) {
          isyslog("live: remote timer '%s' on server '%s' toggled", command.c_str(), timerData.remote);
        }
        else {
          StoreError(timerData, tr("Error in timer settings"));
        }
      }
      response.Clear();
    }
    else {           // toggle local timer
#ifdef DEBUG_LOCK
      dsyslog("live: timers.cpp TimerManager::DoToggleTimer() LOCK_TIMERS_WRITE");
#endif
      LOCK_TIMERS_WRITE;
      Timers->SetExplicitModify();
      cTimer* toggleTimer = Timers->GetById( timerData.id, timerData.remote );
      if ( toggleTimer == 0 ) {
        StoreError( timerData, tr("Timer not defined") );
        return;
      }

      toggleTimer->OnOff();
      Timers->SetModified();
      isyslog("live: timer %s toggled %s", *toggleTimer->ToDescr(), toggleTimer->HasFlags(tfActive) ? "on" : "off");
    }
  }

  void TimerManager::StoreError( timerStruct const& timerData, std::string const& error )
  {
    m_failedUpdates.push_back( ErrorPair( timerData, error ) );
  }

  std::string TimerManager::GetError( timerStruct const& timerData )
  {
    for ( ErrorList::iterator error = m_failedUpdates.begin(); error != m_failedUpdates.end(); ++error ) {
      if ( error->first.id == timerData.id &&
           error->first.remote == timerData.remote &&
           error->first.oldRemote == timerData.oldRemote &&
           error->first.builder == timerData.builder ) {
        std::string message = error->second;
        m_failedUpdates.erase( error );
        return message;
      }
    }
    return "";
  }

  const cTimer* TimerManager::GetTimer(const cEvent *event, const cChannel *channel, const cTimers *Timers)
  {
    if (!event || !channel) return nullptr;
    for (const cTimer* timer = Timers->First(); timer; timer = Timers->Next(timer)) {
      if (timer->Channel() == channel && (timer->Event() == event || timer->Matches(event) == tmFull))
        return timer;
    }
    return nullptr;
  }

  const cTimer* TimerManager::GetTimer(tEventID eventid, tChannelID channelid, const cTimers *Timers)
  {
    LOCK_CHANNELS_READ;
    const cChannel *channel = Channels->GetByChannelID(channelid);
    if (!channel) return nullptr;
    LOCK_SCHEDULES_READ;
    const cSchedule *schedule = Schedules->GetSchedule(channel);
    if (!schedule) return nullptr;
#if APIVERSNUM >= 20502
    const cEvent *event = schedule->GetEventById(eventid);
#else
    const cEvent *event = schedule->GetEvent(eventid);
#endif
    return GetTimer(event, channel, Timers);
  }

} // namespace vdrlive
