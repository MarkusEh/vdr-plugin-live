#include "timers.h"

static bool operator<( cTimer & left, cTimer & right )
{
	return left.Compare( right ) < 0;
}

namespace vdrlive {

SortedTimers::SortedTimers()
{
	for ( cTimer* timer = Timers.First(); timer != 0; timer = Timers.Next( timer ) ) {
		m_timers.push_back( *timer );
	}
	m_timers.sort();
}

} // namespace vdrlive
