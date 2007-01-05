#include <sstream>
#include <vector>
#include "timers.h"
#include "tools.h"

static bool operator<( cTimer const& left, cTimer const& right )
{
	return left.Compare( right ) < 0;
}

namespace vdrlive {

using namespace std;
using namespace vdrlive;

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

cTimer* SortedTimers::GetByTimerId( string const& timerid )
{
	vector< string > parts = StringSplit( timerid, ':' );
	if ( parts.size() < 5 ) {
		esyslog("GetByTimerId: invalid format %s", timerid.c_str() );
		return 0;
	}

	cChannel* channel = Channels.GetByChannelID( tChannelID::FromString( parts[0].c_str() ) );
	if ( channel == 0 ) {
		esyslog("GetByTimerId: no channel %s", parts[0].c_str() );
		return 0;
	}

	try {
		int weekdays = lexical_cast< int >( parts[1] );
		time_t day = lexical_cast< time_t >( parts[2] );
		int start = lexical_cast< int >( parts[3] );
		int stop = lexical_cast< int >( parts[4] );

		for ( SortedTimers::iterator timer = begin(); timer != end(); ++timer ) {
			if ( timer->Channel() == channel && 
					( ( weekdays != 0 && timer->WeekDays() == weekdays ) || ( weekdays == 0 && timer->Day() == day ) ) &&
					timer->Start() == start && timer->Stop() == stop )
				return &*timer;
		}
	} catch ( bad_lexical_cast const& ex ) {
		esyslog("GetByTimer: bad cast");
	}
	return 0;
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

TimerManager& LiveTimerManager()
{
	static TimerManager instance;
	return instance;
}

} // namespace vdrlive
