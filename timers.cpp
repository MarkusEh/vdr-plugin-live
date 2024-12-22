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

  SortedTimers::SortedTimers() { }

  std::string SortedTimers::GetTimerId(cTimer const& timer)
  {
    return std::string(cToSvConcat(timer.Channel()->GetChannelID(), ':', timer.WeekDays(), ':', timer.Day(), ':', timer.Start(), ':', timer.Stop()) );
  }

  const cTimer* SortedTimers::GetByTimerId(cSv timerid)
  {
    std::vector<std::string> parts = StringSplit( timerid, ':' );
    if ( parts.size() < 5 ) {
      esyslog("live: GetByTimerId: invalid format %.*s", (int)timerid.length(), timerid.data() );
      return 0;
    }

#ifdef DEBUG_LOCK
    dsyslog("live: timers.cpp SortedTimers::GetByTimerId() LOCK_TIMERS_READ");
    dsyslog("live: timers.cpp SortedTimers::GetByTimerId() LOCK_CHANNELS_READ");
#endif
    LOCK_TIMERS_READ
    LOCK_CHANNELS_READ;
    const cChannel* channel = Channels->GetByChannelID( tChannelID::FromString( parts[0].c_str() ) );
    if ( channel == 0 ) {
      esyslog("live: GetByTimerId: no channel %s", parts[0].c_str() );
      return 0;
    }

    try {
      int weekdays = parse_int<int>( parts[1] );
      time_t day = parse_int<time_t>( parts[2] );
      int start = parse_int<int>( parts[3] );
      int stop = parse_int<int>( parts[4] );

      cMutexLock MutexLock(&m_mutex);

      for (const cTimer* timer = Timers->First(); timer; timer = Timers->Next(timer)) {
        if ( timer->Channel() == channel &&
           ( ( weekdays != 0 && timer->WeekDays() == weekdays ) || ( weekdays == 0 && timer->Day() == day ) ) &&
           timer->Start() == start && timer->Stop() == stop )
          return timer;
      }
    } catch ( bad_lexical_cast const& ex ) {
      esyslog("live: GetByTimer: bad cast");
    }
    return 0;
  }

  std::string SortedTimers::EncodeDomId(cSv timerid)
  {
    cToSvConcat tId("timer_");
    size_t enc_begin = ((cSv)tId).length();
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
    if (getAutoTimerReason.call(LiveSetup().GetPluginTvscraper()) ) {
      if (!getAutoTimerReason.createdByTvscraper) return "";
      if (getAutoTimerReason.recording) {
        recID = concat("recording_", cToSvXxHash128(getAutoTimerReason.recording->FileName() ));
        recName = std::move(getAutoTimerReason.recordingName);
        return std::move(getAutoTimerReason.reason);
      }
      return concat(getAutoTimerReason.reason, " ", getAutoTimerReason.recordingName);
    }
// fallback information, if this Tvscraper method is not available
    cSv tvScraperInfo = partInXmlTag(timer.Aux(), "tvscraper");
    if (tvScraperInfo.empty()) return "";
    cSv data = partInXmlTag(tvScraperInfo, "reason");
    if (data.empty() ) return "";
    return concat(data, " ", partInXmlTag(tvScraperInfo, "causedBy"));
  }

  bool SortedTimers::Modified()
  {
    bool modified = false;

    // will return != 0 only, if the Timers List has been changed since last read
    if (cTimers::GetTimersRead(m_TimersStateKey)) {
      modified = true;
      m_TimersStateKey.Remove();
    }

    return modified;
  }

  TimerManager::TimerManager()
  :  m_reloadTimers(true)
  {
  }

  void TimerManager::UpdateTimer( int timerId, const char* remote, const char* oldRemote, const tChannelID& channel, cStr builder)
  {
    dsyslog("live: UpdateTimer() timerId '%d'", timerId);
    dsyslog("live: UpdateTimer() remote '%s'", remote);
    dsyslog("live: UpdateTimer() oldRemote '%s'", oldRemote);
    dsyslog("live: UpdateTimer() channel '%s'", *(channel.ToString()));
    dsyslog("live: UpdateTimer() channel '%s'", cToSvConcat(channel).c_str() );
    dsyslog("live: UpdateTimer() builder '%s'", builder.c_str());

    timerStruct timerData = { .id = timerId, .remote=remote, .oldRemote=oldRemote, .builder=std::string(builder) };

    cMutexLock lock( this );
    // dsyslog("live: SV: in UpdateTimer");
    m_updateTimers.push_back( timerData );
    // dsyslog("live: SV: wait for update");
    m_updateWait.Wait( *this );
    // dsyslog("live: SV: update done");

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::DelTimer( int timerId, const char* remote )
  {
    cMutexLock lock( this );

    dsyslog("live: DelTimer() timerId '%d'", timerId);
    dsyslog("live: DelTimer() remote '%s'", remote);

    timerStruct timerData{ .id=timerId, .remote=remote, .oldRemote=remote, .builder=TIMER_DELETE };

    m_updateTimers.push_back( timerData );
    m_updateWait.Wait( *this );

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::ToggleTimerActive( int timerId, const char* remote)
  {
    cMutexLock lock( this );

    timerStruct timerData{ .id=timerId, .remote=remote, .oldRemote=remote, .builder=TIMER_TOGGLE };

    m_updateTimers.push_back( timerData );
    m_updateWait.Wait( *this );

    std::string error = GetError( timerData );
    if ( !error.empty() )
      throw HtmlError( error );
  }

  void TimerManager::DoPendingWork()
  {
    if ( m_updateTimers.size() == 0 && !m_timers.Modified() && !m_reloadTimers )
      return;

    cMutexLock lock( this );
    if ( m_updateTimers.size() > 0 ) {
      DoUpdateTimers();
    }
    // dsyslog("live: SV: signaling waiters");
    m_updateWait.Broadcast();
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
      if ( checkTimer ) {
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
#ifdef TFEVENT
      if (oldTimer->HasFlags(tfEvent) && oldTimer->Start() == copy.Start() && oldTimer->Stop() == copy.Stop() ) copy.SetFlags(tfEvent);
#endif
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
#if VDRVERSNUM < 20301
      if ( Timers.BeingEdited() ) {
        StoreError( timerData, tr("Timers are being edited - try again later") );
        return;
      }
#endif

#if VDRVERSNUM >= 20301
  #ifdef DEBUG_LOCK
                        dsyslog("live: timers.cpp TimerManager::DoToggleTimer() LOCK_TIMERS_WRITE");
  #endif
      LOCK_TIMERS_WRITE;
      Timers->SetExplicitModify();
      cTimer* toggleTimer = Timers->GetById( timerData.id, timerData.remote );
#else
      cTimer* toggleTimer = Timers.GetTimer( const_cast<cTimer*>(timerData.first) );
#endif
      if ( toggleTimer == 0 ) {
        StoreError( timerData, tr("Timer not defined") );
        return;
      }

#if VDRVERSNUM >= 20301
      toggleTimer->OnOff();
      Timers->SetModified();
#else
      toggleTimer->OnOff();
      Timers.SetModified();
#endif
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

  const cTimer* TimerManager::GetTimer(const cEvent *event, const cChannel *channel)
  {
    if (!channel) {
      tChannelID channelid = event->ChannelID();
      if (channelid == tChannelID() ) return nullptr;
      {
#ifdef DEBUG_LOCK
        dsyslog("live: timers.cpp TimerManager::GetTimer() LOCK_CHANNELS_READ");
#endif
        LOCK_CHANNELS_READ;
        channel = Channels->GetByChannelID(channelid);
      }
    }
    if (!channel) return nullptr;

    cMutexLock timersLock( &LiveTimerManager() );
#ifdef DEBUG_LOCK
    dsyslog("live: timers.cpp TimerManager::GetTimer() LOCK_TIMERS_READ");
#endif
    LOCK_TIMERS_READ;
    for (const cTimer* timer = Timers->First(); timer; timer = Timers->Next(timer)) {
      if (timer->Channel() == channel && (timer->Event() == event || timer->Matches(event) == tmFull))
        return timer;
    }
    return nullptr;
  }

  const cTimer* TimerManager::GetTimer(tEventID eventid, tChannelID channelid)
  {
    const cSchedule *schedule;
    {
      LOCK_SCHEDULES_READ;
      schedule = Schedules->GetSchedule(channelid);
    }
    if (!schedule) return nullptr;
#if APIVERSNUM >= 20502
    const cEvent *event = schedule->GetEventById(eventid);
#else
    const cEvent *event = schedule->GetEvent(eventid);
#endif
    if (!event) return nullptr;
    return GetTimer(event);
  }

  TimerManager& LiveTimerManager()
  {
    static TimerManager instance;
    return instance;
  }

} // namespace vdrlive
