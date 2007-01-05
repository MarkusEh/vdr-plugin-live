#include <sstream>
#include "timers.h"

static bool operator<( cTimer const& left, cTimer const& right )
{
	return left.Compare( right ) < 0;
}

namespace vdrlive {

using namespace std;

SortedTimers::SortedTimers():
		m_state( 0 )
{
	ReloadTimers( true );
}

string SortedTimers::GetTimerId( cTimer const& timer )
{
	ostringstream builder;
	builder << *timer.Channel()->GetChannelID().ToString() << ":" << timer.WeekDays() << ":"
			<< timer.Day() << ":" << timer.Start() << ":" << timer.Stop();
	return builder.str();
}

void SortedTimers::ReloadTimers( bool initial )
{
	if ( !Timers.Modified( m_state ) && !initial )
		return;

	dsyslog("reloading timers");

	clear();
	for ( cTimer* timer = Timers.First(); timer != 0; timer = Timers.Next( timer ) ) {
		push_back( *timer );
	}
	sort();
}

TimerManager::TimerManager()
{
}

void TimerManager::DoPendingWork()
{
	cMutexLock lock( this );
	m_timers.ReloadTimers();
}

} // namespace vdrlive
