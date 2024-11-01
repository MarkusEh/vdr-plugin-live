#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

// STL headers need to be before VDR tools.h (included by <vdr/timers.h>)
#include <list>
#include <string>

#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/timers.h>
#include "stringhelpers.h"

namespace vdrlive {

  class SortedTimers: public std::list<cTimer>
  {
    friend class TimerManager;

    public:
      static std::string GetTimerId(cTimer const& timer);
      const cTimer* GetByTimerId(cSv timerid);

      // en- or decodes a timer into an id usable for DOM Ids.
      static std::string EncodeDomId(cSv timerid);
      static std::string DecodeDomId(cSv timerDomId);

      bool Modified();

      static std::string GetTimerDays(cTimer const *timer);
      static std::string GetTimerInfo(cTimer const& timer);
template<std::size_t N>
      static cSv SearchTimerInfo(cTimer const& timer, const char (&value)[N] ) {
        return partInXmlTag(partInXmlTag(timer.Aux(), "epgsearch"), value);
      }
      static std::string TvScraperTimerInfo(cTimer const& timer, std::string &recID, std::string &recName);

    private:
      SortedTimers();
      SortedTimers( SortedTimers const& );

      cMutex m_mutex;
      cStateKey m_TimersStateKey;
  };

  class TimerManager: public cMutex
  {
    friend TimerManager& LiveTimerManager();

    public:
      SortedTimers& GetTimers() { return m_timers; }

      void UpdateTimer( int timerId, const char* remote, const char* oldRemote, const tChannelID& channel, cStr builder);

      void DelTimer( int timerId, const char* remote);
      void ToggleTimerActive( int timerId, const char* remote);
      // may only be called from Plugin::MainThreadHook
      void DoPendingWork();
      const cTimer* GetTimer(tEventID eventid, tChannelID channelid);
      const cTimer* GetTimer(const cEvent *event, const cChannel *channel = nullptr);
      void SetReloadTimers() { m_reloadTimers = true; }

    private:
      typedef struct
                        {
                          int id;
                          const char* remote;
                          const char* oldRemote;
        std::string builder;
                        } timerStruct;

      typedef std::pair<timerStruct, std::string> ErrorPair;
      typedef std::list<timerStruct> TimerList;
      typedef std::list<ErrorPair> ErrorList;

      TimerManager();
      TimerManager( TimerManager const& );

      SortedTimers m_timers;
      TimerList m_updateTimers;
      ErrorList m_failedUpdates;
      cCondVar m_updateWait;
      bool m_reloadTimers = false;

      void DoUpdateTimers();
      void DoInsertTimer( timerStruct& timerData );
      void DoUpdateTimer( timerStruct& timerData );
      void DoDeleteTimer( timerStruct& timerData );
      void DoToggleTimer( timerStruct& timerData );

      void StoreError( timerStruct const& timerData, std::string const& error );
      std::string GetError( timerStruct const& timerData );
  };

  TimerManager& LiveTimerManager();

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
