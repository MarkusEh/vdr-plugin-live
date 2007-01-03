#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

#include <list>
#include <vdr/timers.h>

namespace vdrlive {

class SortedTimers
{
public:
	typedef std::list< cTimer > List;
	typedef List::iterator iterator;
	
	SortedTimers();

	iterator begin() { return m_timers.begin(); }
	iterator end() { return m_timers.end(); }
	
private:
	List m_timers;
};

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
