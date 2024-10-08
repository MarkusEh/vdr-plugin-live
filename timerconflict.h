#ifndef VDR_LIVE_TIMERCONFLICT_H
#define VDR_LIVE_TIMERCONFLICT_H

#include "stdext.h"

#include <list>
#include <string>
#include <time.h>

// #include <vdr/timers.h> avoid this to ensure the correct order of vdr includes / cxx includes, see https://www.vdr-portal.de/forum/index.php?thread/135369-live-3-1-12/&postID=1359294#post1359294

namespace vdrlive {

// classes for timer conflict interface

// conflicting timer
  class TimerInConflict
  {
    public:
      int timerIndex;               // it's index in VDR
      std::string remote = "";
      int percentage;                // percentage of recording
      std::list<int> concurrentTimerIndices;    // concurrent timer indices

      TimerInConflict(int TimerIndex=-1, int Percentage=0) : timerIndex(TimerIndex), percentage(Percentage) {}
  };

  class TimerConflict;

  bool operator<( TimerConflict const& left, TimerConflict const& right );

// one timer conflict time
  class TimerConflict
  {
      time_t conflictTime;              // time of conflict
      std::list<TimerInConflict> conflictingTimers; // conflicting timers at this time

    friend bool operator<( TimerConflict const& left, TimerConflict const& right );

    public:
      TimerConflict( std::string const& data );
      TimerConflict();
      void Init();

      time_t ConflictTime() { return conflictTime; }
      const std::list<TimerInConflict>& ConflictingTimers() const { return conflictingTimers; }
  };

  class TimerConflicts
  {
    public:
      typedef std::list<TimerConflict> ConflictList;
      typedef ConflictList::size_type size_type;
      typedef ConflictList::iterator iterator;
      typedef ConflictList::const_iterator const_iterator;

      TimerConflicts();

      size_type size() const { return m_conflicts.size(); }
      iterator begin() { return m_conflicts.begin(); }
      const_iterator begin() const { return m_conflicts.begin(); }
      iterator end() { return m_conflicts.end(); }
      const_iterator end() const { return m_conflicts.end(); }

      bool HasConflict(int timerId);
      static bool CheckAdvised();
    private:
      void GetRemote(std::list<std::string> & conflicts);
      ConflictList m_conflicts;
  };

  class TimerConflictNotifier
  {
    public:
      typedef std::shared_ptr<TimerConflicts> TimerConflictsPtr;

      TimerConflictNotifier();
      virtual ~TimerConflictNotifier();

      bool ShouldNotify();

      void SetTimerModification();

      std::string Message() const;
      std::string Url() const;

      TimerConflictsPtr const CurrentConflicts() const { return conflicts; }

      static int const CHECKINTERVAL = 5; // recheck value in seconds.

    private:
      time_t lastCheck;
      time_t lastTimerModification;
      TimerConflictsPtr conflicts;
  }; // class TimerConflictNotifier

} // namespace vdrlive

#endif // VDR_LIVE_TIMERCONFLICT_H

