#include "sortedtimers.h"

static bool operator<( cTimer const& left, cTimer const& right )
{
	return left.Compare( right ) < 0;
}

/*
static bool operator==( cTimer const& left, cTimer const& right )
{
	return left.Channel() == right.Channel() &&
          (left.WeekDays() && left.WeekDays() == right.WeekDays() || !left.WeekDays() && left.Day() == right.Day()) &&
		   left.Start() == right.Start() &&
		   left.Stop() == right.Stop();
}
*/

namespace vdrlive {

SortedTimers::SortedTimers()
{
	for ( cTimer* timer = Timers.First(); timer != 0; timer = Timers.Next( timer ) ) {
		m_timers.push_back( *timer );
	}
	m_timers.sort();
}

} // namespace vdrlive
